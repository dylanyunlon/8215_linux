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

/*-----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "aud_oal.h"
#include "drv_dsp_cfg.h"
#include "aud_debug.h"
#include "aud_drv.h"
#include "DspDrvInc.h"
#include "aud_if.h"
#include "DspFunc.h"
#include <media/atc/x_audin.h>
#include <media/atc/aud_output.h>
#include <media/atc/drv_esm_if.h>

/*-----------------------------------------------------------------------------
Local Variable definitions
----------------------------------------------------------------------------*/
#define MAX_VOL_GAIN   ((u32) 0x20000)
#define MAX_VOL_LEVEL   ((u8) 100)

CLICmd g_rCLICmd1= {0};

AUD_DEC_REAR_VOLUME_GAIN_INFO_T g_rRearVolGain = {MAX_VOL_GAIN};
AUD_DEC_CH_VOL_GAIN_T g_rFrnVolGain =
{
    MAX_VOL_GAIN,
    MAX_VOL_GAIN,
    MAX_VOL_GAIN,
    MAX_VOL_GAIN,
    MAX_VOL_GAIN,
    MAX_VOL_GAIN,
    MAX_VOL_GAIN
};//for vol table arrage to app
static AUD_DEC_CH_VOL_T g_rFrnVolVal =
{
    MAX_VOL_LEVEL,
    MAX_VOL_LEVEL,
    MAX_VOL_LEVEL,
    MAX_VOL_LEVEL,
    MAX_VOL_LEVEL,
    MAX_VOL_LEVEL,
    MAX_VOL_LEVEL
};
u32 g_rSrcVolume[AUD_MEDIA_SOURCE_DVP + 1][AUD_MEDIA_OUT_REAR + 1] = {0x20000, 0x20000, 0x20000, 0x20000, 0x20000, 0x20000,
                                                                      0x20000, 0x20000, 0x20000, 0x20000, 0x20000, 0x20000};
bool g_rSrcMute[AUD_MEDIA_SOURCE_DVP + 1][AUD_MEDIA_OUT_REAR + 1] = {false, false, false, false, false, false,
                                                                      false, false, false, false, false, false};
extern struct semaphore g_aud_volume_lock;

static AUD_DEC_FMT_INFO_T  g_rAudDecFmt[TER_DEC + 1];
static AUD_DEC_CTRL_T      g_eCtrlSetting[TER_DEC + 1] = {AUD_DEC_CTRL_STOP};
//static AUD_DEC_MUTE_TYPE_T g_eMuteType = AUD_DEC_MUTE_OFF;
static bool g_fgPolicyStatus = FALSE;

extern void vAdspChannelVolumeCtrl(AUD_CH_T eChannel, u8 u1Value);
extern void vAdspMasterVolume(u8 u1Volume);
extern void vAdspChannelVolumeGainCtrl(AUD_CH_T eChannel, u32 u4Value);
extern void vAdspMasterVolumeGain(u32 u4Volume);
extern void vAdspSrcVolumeCtrl(AUD_MEDIA_SOURCE_TYPE_T eSrc, AUD_MEDIA_OUT_TYPE_T eType, u32 u4Value);

void AudSetMasterVolPolicy(AUD_VOL_POLICY_T eType, u32 u4VolGain);


/*-----------------------------------------------------------------------------
Local Function definitions
----------------------------------------------------------------------------*/

static void AudCheckMwCmd(u32 mwcmd)
{
   CLICmd *ptCmd = &g_rCLICmd1;
   if (ptCmd->wrptr >= 128)
   {
        VERIFY(0);
   }
   else
   {
        ptCmd->mw_cmd[ptCmd->wrptr]=mwcmd;
   }
   ptCmd->wrptr++;

   if (ptCmd->wrptr >= 128)
   {
        ptCmd->wrptr = 0;
   }
}

void AudGetRearVolGain(AUD_DEC_REAR_VOLUME_GAIN_INFO_T * prChannelVolGain)
{
    prChannelVolGain->u4RearVolGain = g_rRearVolGain.u4RearVolGain;
}

void AudGetFrnVolGain(AUD_DEC_VOLUME_GAIN_INFO_T * prChannelVolGain)
{
    if (prChannelVolGain->e_vol_type == AUD_DEC_ALL_CH)
    {
        prChannelVolGain->u.u4FrontMasterVolGain = g_rFrnVolGain.u4VolMaster;
    }
    else /* AUD_DEC_INDIVIDUAL_CH */
    {
        switch (prChannelVolGain->u.t_ch_gain_vol.e_ls)
        {
        case AUD_DEC_LS_FRONT_LEFT:
            prChannelVolGain->u.t_ch_gain_vol.u4FrontChVolGain = g_rFrnVolGain.u4VolL;
            break;

        case AUD_DEC_LS_FRONT_RIGHT:
            prChannelVolGain->u.t_ch_gain_vol.u4FrontChVolGain = g_rFrnVolGain.u4VolR;
            break;

        case AUD_DEC_LS_REAR_LEFT:
            prChannelVolGain->u.t_ch_gain_vol.u4FrontChVolGain = g_rFrnVolGain.u4VolSL;
            break;

        case AUD_DEC_LS_REAR_RIGHT:
            prChannelVolGain->u.t_ch_gain_vol.u4FrontChVolGain = g_rFrnVolGain.u4VolSR;
            break;

        case AUD_DEC_LS_CENTER:
            prChannelVolGain->u.t_ch_gain_vol.u4FrontChVolGain = g_rFrnVolGain.u4VolC;
            break;

        case AUD_DEC_LS_SUB_WOOFER:
            prChannelVolGain->u.t_ch_gain_vol.u4FrontChVolGain = g_rFrnVolGain.u4VolSW;
            break;

        default:
            break;
        }
    }
}


void AudGetFrnVolume(AUD_DEC_VOLUME_INFO_T * prChannelVol)
{
    if (prChannelVol->e_vol_type == AUD_DEC_ALL_CH)
    {
        prChannelVol->u.ui1_level = g_rFrnVolVal.u1VolMaster;
    }
    else /* AUD_DEC_INDIVIDUAL_CH */
    {
        switch (prChannelVol->u.t_ch_vol.e_ls)
        {
        case AUD_DEC_LS_FRONT_LEFT:
            prChannelVol->u.t_ch_vol.ui1_level = g_rFrnVolVal.u1VolL;
            break;

        case AUD_DEC_LS_FRONT_RIGHT:
            prChannelVol->u.t_ch_vol.ui1_level = g_rFrnVolVal.u1VolR;
            break;

        case AUD_DEC_LS_REAR_LEFT:
            prChannelVol->u.t_ch_vol.ui1_level = g_rFrnVolVal.u1VolSL;
            break;

        case AUD_DEC_LS_REAR_RIGHT:
            prChannelVol->u.t_ch_vol.ui1_level = g_rFrnVolVal.u1VolSR;
            break;

        case AUD_DEC_LS_CENTER:
            prChannelVol->u.t_ch_vol.ui1_level = g_rFrnVolVal.u1VolC;
            break;

        case AUD_DEC_LS_SUB_WOOFER:
            prChannelVol->u.t_ch_vol.ui1_level = g_rFrnVolVal.u1VolSW;
            break;

        default:
            break;
        }
    }
}


s32 AudSetMwCtrl(u8 u1DecId, AUD_DEC_CTRL_T  eAudCtrl)
{
    s32 s4Ret = AUD_OK;
    VERIFY(u1DecId <= TER_DEC);

    if(eAudCtrl == AUD_DEC_CTRL_RESET)
    {
        g_eCtrlSetting[u1DecId] = AUD_DEC_CTRL_STOP;
    }
    else
    {
        g_eCtrlSetting[u1DecId] = eAudCtrl;
    }

    LOG(LOG_CTRLF, TEXT("Dec=%d:eAudCtrl=%d(0-reset,1-stop,2-play,6-pause,7-resume)\n"), u1DecId, eAudCtrl);
    AudCheckMwCmd(eAudCtrl);

    switch (eAudCtrl)
    {
    case AUD_DEC_CTRL_RESET:
        s4Ret = AUD_DSPCmdResetAsyn(u1DecId);
        break;

    case AUD_DEC_CTRL_STOP:
        s4Ret = AUD_DSPCmdStopAsyn(u1DecId);
        break;

    case AUD_DEC_CTRL_PLAY:
        s4Ret = AUD_DSPCmdPlayAsyn(u1DecId);
        break;

    case AUD_DEC_CTRL_PLAY_SYNC:
        AudDrvSetAvSynMode(u1DecId, AV_SYNC_SLAVE);
        s4Ret = AUD_DSPCmdPlayAsyn(u1DecId);
        break;

    case AUD_DEC_CTRL_FLUSH:
        s4Ret = AUD_DSPCmdStopAsyn(u1DecId);
        break;

    case AUD_DEC_CTRL_PAUSE:
        s4Ret = AUD_DSPCmdPauseAsyn(u1DecId);
        break;

    case AUD_DEC_CTRL_RESUME:
        s4Ret = AUD_DSPCmdResumeAsyn(u1DecId);
        break;

    default:
        LOG(LOG_CTRLF, TEXT("SetCtrl Error\n"));
        s4Ret = AUD_FAIL;
        break;
    }
    return (s4Ret);
}

bool AudSetFormat(u8 u1DecId, AUD_DRV_FMT_INFO_T * prFormatInfo)
{
    bool fgRet = TRUE;
    AUD_DRV_STREAM_FROM_T eStreamFrom = AUD_STREAM_FROM_MEMORY;
    LOG(LOG_CTRLF, TEXT("Dec(%d) Audio Format = 0x%x.\n"), u1DecId, prFormatInfo->e_fmt);
    g_rAudDecFmt[u1DecId].e_fmt = (AUD_DEC_FMT_T)prFormatInfo->e_fmt;
    if(AUD_OK != AUD_SetDecType(u1DecId, eStreamFrom, prFormatInfo))
    {
        fgRet = FALSE;
    }

    return (fgRet);
}

void vAudAddLastFrame(void)
{
    if((g_rAudDecFmt[PRI_DEC].e_fmt == AUD_DEC_FMT_MP3) || (g_rAudDecFmt[PRI_DEC].e_fmt == AUD_DEC_FMT_AAC_PURE))
    {
		AUD_SEND_BUF_INFO prLastPkt;
		prLastPkt.ptrBufAddr = (AUD_SEND_BUF_INFO *)kzalloc(5000, GFP_KERNEL);
		prLastPkt.u4BufLen = 5000;
		prLastPkt.u8Pts = 0;
		if(NULL != prLastPkt.ptrBufAddr)
		{
			memset(prLastPkt.ptrBufAddr, 0x0, 5000);
			u8 *u1PtrTmp = (u8 *)prLastPkt.ptrBufAddr;
			if(g_rAudDecFmt[PRI_DEC].e_fmt == AUD_DEC_FMT_MP3)
			{
				*u1PtrTmp++ = 0xFF;
				*u1PtrTmp++ = 0xFB;
				*u1PtrTmp++ = 0x92;
				*u1PtrTmp++ = 0xC4;
				LOG(LOG_CTRLF, TEXT("AUD_DEC_FMT_MP3!!!\n"));
			}
			else if(g_rAudDecFmt[PRI_DEC].e_fmt == AUD_DEC_FMT_AAC_PURE)
			{
				*u1PtrTmp++ = 0xFF;
				*u1PtrTmp++ = 0xF1;
				*u1PtrTmp++ = 0x50;
				*u1PtrTmp++ = 0x40;
				*u1PtrTmp++ = 0x01;
				*u1PtrTmp++ = 0xBF;
				*u1PtrTmp++ = 0xFC;
				LOG(LOG_CTRLF, TEXT("AUD_DEC_FMT_AAC_PURE!!!\n"));
			}

			AudEsm_SendBufferInfo(PRI_DEC, &prLastPkt, FALSE);
			kfree(prLastPkt.ptrBufAddr);
			prLastPkt.ptrBufAddr = NULL;
		}
    }
}

void AudSetFrnVolGain(AUD_DEC_VOLUME_GAIN_INFO_T * prChlVolGain, AUD_VOL_POLICY_T eType)
{
    if (prChlVolGain->e_vol_type == AUD_DEC_ALL_CH)
    {
        AudSetMasterVolPolicy(eType, prChlVolGain->u.u4FrontMasterVolGain);
    }
    else if (prChlVolGain->e_vol_type == AUD_DEC_INDIVIDUAL_CH)
    {
        AUD_CH_T rAudCh = (AUD_CH_T)prChlVolGain->u.t_ch_gain_vol.e_ls;

        LOG(LOG_CTRLF, TEXT("rAudCh = 0x%x, volume gain = 0x%x\r\n"),
            rAudCh, prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain );

        switch (rAudCh)
        {
        case AUD_DEC_LS_FRONT_LEFT:
            if(prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain != g_rFrnVolGain.u4VolL)
            {
                g_rFrnVolGain.u4VolL = prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain;
                vAdspChannelVolumeGainCtrl(rAudCh, g_rFrnVolGain.u4VolL);
            }
            break;

        case AUD_DEC_LS_FRONT_RIGHT:
            if(prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain != g_rFrnVolGain.u4VolR)
            {
                g_rFrnVolGain.u4VolR = prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain;
                vAdspChannelVolumeGainCtrl(rAudCh, g_rFrnVolGain.u4VolR);
            }
            break;

        case AUD_DEC_LS_REAR_LEFT:
            if(prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain != g_rFrnVolGain.u4VolSL)
            {
                g_rFrnVolGain.u4VolSL= prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain;
                vAdspChannelVolumeGainCtrl(rAudCh, g_rFrnVolGain.u4VolSL);
            }
            break;

        case AUD_DEC_LS_REAR_RIGHT:
            if(prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain != g_rFrnVolGain.u4VolSR)
            {
                g_rFrnVolGain.u4VolSR= prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain;
                vAdspChannelVolumeGainCtrl(rAudCh, g_rFrnVolGain.u4VolSR);
            }
            break;

        case AUD_DEC_LS_CENTER:
            if(prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain != g_rFrnVolGain.u4VolC)
            {
                g_rFrnVolGain.u4VolC= prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain;
                vAdspChannelVolumeGainCtrl(rAudCh, g_rFrnVolGain.u4VolC);
            }
            break;

        case AUD_DEC_LS_SUB_WOOFER:
            if(prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain != g_rFrnVolGain.u4VolSW)
            {
                g_rFrnVolGain.u4VolSW= prChlVolGain->u.t_ch_gain_vol.u4FrontChVolGain;
                vAdspChannelVolumeGainCtrl(rAudCh, g_rFrnVolGain.u4VolSW);
            }
            break;

        default:
            LOG(LOG_FAIL, TEXT("Trim adjust unsupport speaker.\r\n"));
            break;
        }
    }
}

void AudSetSrcVolGain(AUD_SRC_VOL_CTL * prSrcVol)
{
    u16 u2UopIndex = 0;
    VERIFY( prSrcVol->u4Vol >= 0);
    if (prSrcVol->eMediaSrc >= AUD_MEDIA_SOURCE_UNDEF || prSrcVol->eMediaOut >= AUD_MEDIA_OUT_UNDEF )
    {
        LOG(LOG_FAIL, TEXT("MediaSrc:%d or eMediaOut:%d type error!\r\n"),prSrcVol->eMediaSrc ,prSrcVol->eMediaOut);
        return;
    }

    AUD_MEDIA_SOURCE_TYPE_T  rAudSrc = (AUD_MEDIA_SOURCE_TYPE_T)(prSrcVol->eMediaSrc);
    AUD_MEDIA_OUT_TYPE_T rAudOutType = (AUD_MEDIA_OUT_TYPE_T)(prSrcVol->eMediaOut);
    u32 u4Vol = prSrcVol->u4Vol;

    down(&g_aud_volume_lock);

    if (g_rSrcVolume[rAudSrc][rAudOutType] != prSrcVol->u4Vol && false == g_rSrcMute[rAudSrc][rAudOutType])
    {
        g_rSrcVolume[rAudSrc][rAudOutType] = prSrcVol->u4Vol;
        vAdspSrcVolumeCtrl(rAudSrc, rAudOutType, u4Vol);
    }
    else
    {
        g_rSrcVolume[rAudSrc][rAudOutType] = prSrcVol->u4Vol;
        LOG(LOG_FAIL, TEXT("g_rSrcVolume[%d][%d] equal to prSrcVol->u4Vol, the value is 0x%x, or muted!\r\n"),
                rAudSrc, rAudOutType, prSrcVol->u4Vol);
    }
    up(&g_aud_volume_lock);
}

void AudGetSrcVolGain(AUD_SRC_VOL_CTL * prSrcVol, u32* pGetVolume)
{
    u16 u2UopIndex = 0;
    VERIFY( prSrcVol->u4Vol >= 0);

    if (prSrcVol->eMediaSrc >= AUD_MEDIA_SOURCE_UNDEF || prSrcVol->eMediaOut >= AUD_MEDIA_OUT_UNDEF )
    {
        LOG(LOG_FAIL, TEXT("AudGetSrcVolGain invalid parameter!.\r\n"));
        return;
    }

    AUD_MEDIA_SOURCE_TYPE_T  rAudSrc = (AUD_MEDIA_SOURCE_TYPE_T)(prSrcVol->eMediaSrc);
    AUD_MEDIA_OUT_TYPE_T rAudOutType = (AUD_MEDIA_OUT_TYPE_T)(prSrcVol->eMediaOut);
    down(&g_aud_volume_lock);
    *pGetVolume = g_rSrcVolume[rAudSrc][rAudOutType];
    up(&g_aud_volume_lock);
    return;

}

void AudSetSrcMute(AUD_SRC_MUTE_CTL * prSrcMute)
{
    if (NULL == prSrcMute)
	{
	    LOG(LOG_FAIL, TEXT("AudSetSrcMute prSrcMute == NULL!.\r\n"));
            return;
	}

    if (prSrcMute->eMediaSrc >= AUD_MEDIA_SOURCE_UNDEF || prSrcMute->eMediaOut >= AUD_MEDIA_OUT_UNDEF )
    {
	    LOG(LOG_FAIL, TEXT("AudSetSrcMute invalid parameter!.\r\n"));
	    return;
    }
    AUD_MEDIA_SOURCE_TYPE_T  rAudSrc = (AUD_MEDIA_SOURCE_TYPE_T)(prSrcMute->eMediaSrc);
    AUD_MEDIA_OUT_TYPE_T rAudOutType = (AUD_MEDIA_OUT_TYPE_T)(prSrcMute->eMediaOut);
    down(&g_aud_volume_lock);
    if (true == prSrcMute->fgMute && false == g_rSrcMute[rAudSrc][rAudOutType])
    {
        g_rSrcMute[rAudSrc][rAudOutType] = true;
        if (0 != g_rSrcVolume[rAudSrc][rAudOutType])
        {
            int tempVolume = g_rSrcVolume[rAudSrc][rAudOutType];
            g_rSrcVolume[rAudSrc][rAudOutType] = 0;
            vAdspSrcVolumeCtrl(rAudSrc, rAudOutType, 0);
            Sleep(50);
            g_rSrcVolume[rAudSrc][rAudOutType] = tempVolume;
        }
    } else if(false == prSrcMute->fgMute && true == g_rSrcMute[rAudSrc][rAudOutType]){
        vAdspSrcVolumeCtrl(rAudSrc, rAudOutType, g_rSrcVolume[rAudSrc][rAudOutType]);
        g_rSrcMute[rAudSrc][rAudOutType] = false;
	}else {
	    LOG(LOG_FAIL, TEXT("AudSetSrcMute no need to change mute state\r\n"));
	}
	up(&g_aud_volume_lock);
}

void AudSetFrnVolume(AUD_DEC_VOLUME_INFO_T * prChlVol)
{
    if (prChlVol->e_vol_type == AUD_DEC_ALL_CH)
    {
        g_rFrnVolVal.u1VolMaster = prChlVol->u.ui1_level;
        vAdspMasterVolume(prChlVol->u.ui1_level);
    }
    else if (prChlVol->e_vol_type == AUD_DEC_INDIVIDUAL_CH)
    {
        AUD_CH_T rAudCh = (AUD_CH_T)prChlVol->u.t_ch_vol.e_ls;
        switch (prChlVol->u.t_ch_vol.e_ls)
        {
        case AUD_DEC_LS_FRONT_LEFT:
            g_rFrnVolVal.u1VolL = prChlVol->u.t_ch_vol.ui1_level;
            break;

        case AUD_DEC_LS_FRONT_RIGHT:
            g_rFrnVolVal.u1VolR = prChlVol->u.t_ch_vol.ui1_level;
            break;

        case AUD_DEC_LS_REAR_LEFT:
            g_rFrnVolVal.u1VolSL = prChlVol->u.t_ch_vol.ui1_level;
            break;

        case AUD_DEC_LS_REAR_RIGHT:
            g_rFrnVolVal.u1VolSR = prChlVol->u.t_ch_vol.ui1_level;
            break;

        case AUD_DEC_LS_CENTER:
            g_rFrnVolVal.u1VolC = prChlVol->u.t_ch_vol.ui1_level;
            break;

        case AUD_DEC_LS_SUB_WOOFER:
            g_rFrnVolVal.u1VolSW = prChlVol->u.t_ch_vol.ui1_level;
            break;

        default:
            break;
        }
        vAdspChannelVolumeCtrl(rAudCh, prChlVol->u.t_ch_vol.ui1_level);
    }
}

void AudSetRearVolGain(AUD_DEC_REAR_VOLUME_GAIN_INFO_T * pRearChVol)
{
    u32 u4VolGain = 0;

    LOG(LOG_CTRLF, TEXT("Rear Vol Gain = 0x%x:\r\n"), pRearChVol->u4RearVolGain);

    u4VolGain = pRearChVol->u4RearVolGain;

    if (u4VolGain > MAX_VOL_GAIN)
    {
        u4VolGain = MAX_VOL_GAIN;
    }

    if(g_rRearVolGain.u4RearVolGain != u4VolGain)
    {
        g_rRearVolGain.u4RearVolGain = u4VolGain;
        vAdspRearChVolGainCtrl(u4VolGain);
    }
}

void AudSetRearVolume(AUD_DEC_REAR_VOLUME_INFO_T * pRearChVol)
{
    u32 u4VolumeShm = 0;
    u8  u1Value    = 0;
    u32 u4RearMasterVolumeBase = 0;

    LOG(LOG_FEATURE, TEXT("Rear Vol Level = 0x%x:\n"), pRearChVol->ui1_level);

    u1Value = pRearChVol->ui1_level;

    if (u1Value > MAX_VOL_LEVEL)
    {
        u1Value = MAX_VOL_LEVEL;
    }

    u4RearMasterVolumeBase = MAX_VOL_GAIN/MAX_VOL_LEVEL;

    if (u1Value == MAX_VOL_LEVEL)
    {
        u4VolumeShm = MAX_VOL_GAIN;
    }
    else
    {
        u4VolumeShm = u4RearMasterVolumeBase * u1Value;
    }
    LOG(LOG_FEATURE, TEXT("Rear Vol u4VolumeShm = %x:\n"),u4VolumeShm);

    vAdspRearChVolGainCtrl(u4VolumeShm);
}

void AudSetMute(AUD_DEC1_MUTE_CTRL_T eMuteType)
{
    if(AUD_DEC1_MUTE_OFF == eMuteType)
    {
        vAdspDecMute(FALSE);
    }
    else
    {
        vAdspDecMute(TRUE);
    }
}

void AudSetDec1Mute(AUD_DEC1_MUTE_CTRL_T eDec1MuteCtrl)
{
    switch (eDec1MuteCtrl)
    {
    case AUD_DEC1_MUTE_OFF:
        vAdspDecMute(FALSE);
        break;

    case AUD_DEC1_MUTE_ON:
        vAdspDecMute(TRUE);
        break;

    default:
        break;
    }
}

void AudSetSpdif(AUD_DEC_SPDIF_TYPE_T eSpdif)
{
    LOG(LOG_FEATURE, TEXT("[AUD_MW]Set Spdif type = %d\n"),(u32)eSpdif);
    switch(eSpdif)
    {
    case AUD_DEC_SPDIF_OFF:
        fgAdspIECConfig(PRI_DEC, AUD_IEC_CFG_PCM, FALSE);
        break;
    case AUD_DEC_SPDIF_RAW:
        fgAdspIECConfig(PRI_DEC, AUD_IEC_CFG_RAW, TRUE);
        break;
    case AUD_DEC_SPDIF_PCM:
        fgAdspIECConfig(PRI_DEC, AUD_IEC_CFG_PCM, TRUE);
        break;
    case AUD_DEC_SPDIF_NON_PCM:
        fgAdspIECConfig(PRI_DEC, AUD_IEC_CFG_RAW_REENCODE, TRUE);
        break;
    default:
        LOG(LOG_FAIL, TEXT("[AUD_MW]unsupport spdif type = %d\n"),(u32)eSpdif);
        break;
    }
}

void AudSetSpkCfg(u8 u1DecId,AUD_DEC_SPEAKER_LAYOUT_T rSpeakerLayout)
{
    vAdspSetSpeakerConfig(rSpeakerLayout);
}

void AudSetDrc(u8 u1DecId, AUD_DEC_DRC_T eDrc)
{
    VERIFY(u1DecId < 2);
    vAdspAC3DRCRange((u8)eDrc, u1DecId);
}

static void AudSetDecPCMInfo(AUD_DEC_AUD_INFO_T * pv_set_info)
{
    AUD_DRV_PCM_SETTING_T tPcmSetting;
    AUD_DEC_PCM_INFO_T *ptPcmInfo = NULL;

    x_memset(&tPcmSetting, 0, sizeof(AUD_DRV_PCM_SETTING_T));

    if (pv_set_info == NULL)
    {
        LOG(LOG_ERROR, TEXT("[AUD_MW][Err] Unknown DVD LPCM setting.\n"));
        VERIFY(0);
        return;
    }

    if ((AUD_DEC_PCM_INFO_T*)pv_set_info->u_fmt_spec.pt_pcm_info == NULL)
    {
        return;
    }
    else
    {
        ptPcmInfo = (AUD_DEC_PCM_INFO_T*)pv_set_info->u_fmt_spec.pt_pcm_info;
    }

    if (ptPcmInfo->b_dlna_exist)
    {
        vAudCodecSet_DLNA_Info();
    }

    tPcmSetting.tPcmInfo.ePCM_Format = (AUD_DRV_PCM_FMT_T) ptPcmInfo->ePCM_Format;
    tPcmSetting.u1DecType = (u8) ptPcmInfo->ePCM_Format; // something wrong ??????
    tPcmSetting.tPcmInfo.u2BlockAlign = ptPcmInfo->u2BlockAlign;
    tPcmSetting.u4SamplingFreq = pv_set_info->ui4_sample_rate;

    switch (ptPcmInfo->ePCM_Format)
    {
    case AUD_DEC_PCM_FMT_WAVE:
    case AUD_DEC_PCM_FMT_PCM_DVDV:
    case AUD_DEC_PCM_FMT_PCM_DVDV_2CH:
    case AUD_DEC_PCM_FMT_ADPCM:
    case AUD_DEC_PCM_FMT_PCM_NORMAL:
    case AUD_DEC_PCM_FMT_ADPCM_MS:
        //  Transfer Channel Assignment
        switch (pv_set_info->e_aud_type)
        {
        case AUD_DEC_TYPE_MONO:
            tPcmSetting.u1ChannelAssign = LPCM_DVD_CH_MONO;
            break;
        case AUD_DEC_TYPE_STEREO:
            tPcmSetting.u1ChannelAssign = LPCM_DVD_CH_STEREO;
            break;
        case AUD_DEC_TYPE_SURROUND_2CH: // 0x2
            tPcmSetting.u1ChannelAssign = LPCM_DVD_CH_LF_RF_S;
            break;
        case AUD_DEC_TYPE_SURROUND:  //0xF
            tPcmSetting.u1ChannelAssign = LPCM_DVD_CH_LF_RF_C_LFE;
            break;
        case AUD_DEC_TYPE_3_0: // 0x7
            tPcmSetting.u1ChannelAssign = LPCM_DVD_CH_LF_RF_C;
            break;
        case AUD_DEC_TYPE_4_0: // 0x3
            tPcmSetting.u1ChannelAssign = LPCM_DVD_CH_LF_RF_LS_RS;
            break;
        case AUD_DEC_TYPE_5_0: // 0x9
            tPcmSetting.u1ChannelAssign = LPCM_DVD_CH_LF_RF_LS_RS_C;
            break;
        case AUD_DEC_TYPE_5_1: //0xC
            tPcmSetting.u1ChannelAssign = LPCM_DVD_CH_LF_RF_LS_RS_C_LFE;
            break;
        case AUD_DEC_TYPE_7_1: //0x17
            tPcmSetting.u1ChannelAssign = LPCM_DVD_CH_LF_RF_LS_RS_C_LFE_Ch7_Ch8;
            break;
        default:
            tPcmSetting.u1ChannelAssign = LPCM_DVD_CH_STEREO;
            LOG(5, TEXT("[AUD_MW][Err] Unknown channels of DVD LPCM setting.\n"));
            break;
        }

        // transfer bit resolution
        switch (pv_set_info->ui1_bit_depth)
        {
        case 16:
            tPcmSetting.u1BitResolution = 0;
            break;
        case 20:
            tPcmSetting.u1BitResolution = 1;
            break;
        case 24:
            tPcmSetting.u1BitResolution = 2;
            break;
        case 8:
            tPcmSetting.u1BitResolution = 3;
            break;
        default:
            tPcmSetting.u1BitResolution = 0;
            break;
        }

        tPcmSetting.u1IsBD = FALSE;
        vAudCodecSet_LPCMPara(&tPcmSetting);

        break;
    case AUD_DEC_PCM_FMT_PCM_BD:
        tPcmSetting.u1IsBD = TRUE;
        tPcmSetting.tPcmInfo.u2BlockAlign = 0;
        vAudCodecSet_LPCMPara(&tPcmSetting);
        break;
    default:
        VERIFY(0);
        break;
    }
}

static void AudSetHDMIInPcmInfo(AUD_DEC_AUD_INFO_T * pv_set_info)
{
    AUD_DRV_PCM_SETTING_T tPcmSetting;
    AUD_DEC_PCM_INFO_T *ptPcmInfo = NULL;

    x_memset(&tPcmSetting, 0, sizeof(AUD_DRV_PCM_SETTING_T));

    if (pv_set_info == NULL)
    {
        LOG(LOG_ERROR, TEXT("[AUD_MW][Err] Unknown DVD LPCM setting.\n"));
        VERIFY(0);
        return;
    }

    if ((AUD_DEC_PCM_INFO_T*)pv_set_info->u_fmt_spec.pt_pcm_info == NULL)
    {
        return;
    }
    else
    {
        ptPcmInfo = (AUD_DEC_PCM_INFO_T*)pv_set_info->u_fmt_spec.pt_pcm_info;
    }

    LOG(LOG_ADSP_INFO, TEXT("[AUD_MW]HDMI LPCM setting.\n"));

    if (ptPcmInfo->b_dlna_exist)
    {
        vAudCodecSet_DLNA_Info();
    }              //no break;  continue Channel setting

    tPcmSetting.tPcmInfo.ePCM_Format = (AUD_DRV_PCM_FMT_T) ptPcmInfo->ePCM_Format;
    tPcmSetting.u1DecType = (u8) ptPcmInfo->ePCM_Format; // something wrong ??????
    tPcmSetting.tPcmInfo.u2BlockAlign = ptPcmInfo->u2BlockAlign;
    tPcmSetting.u4SamplingFreq = pv_set_info->ui4_sample_rate;

    //  Transfer Channel Assignment
    switch (pv_set_info->e_aud_type)
    {
    case AUD_DEC_TYPE_STEREO:
        tPcmSetting.u1ChannelAssign = CA_FL_FR;
        break;
    case AUD_DEC_TYPE_STEREO_SUB:
        tPcmSetting.u1ChannelAssign = CA_LFE_FL_FR;
        break;
    case AUD_DEC_TYPE_SURROUND:
        tPcmSetting.u1ChannelAssign = CA_FC_LFE_FL_FR;
        break;
    case AUD_DEC_TYPE_3_0:
        tPcmSetting.u1ChannelAssign = CA_FC_FL_FR;
        break;
    case AUD_DEC_TYPE_4_0:
        tPcmSetting.u1ChannelAssign = CA_RR_RL_FL_FR;
        break;
    case AUD_DEC_TYPE_4_1:
        tPcmSetting.u1ChannelAssign = CA_RR_RL_LFE_FL_FR;
        break;
    case AUD_DEC_TYPE_5_0:
        tPcmSetting.u1ChannelAssign = CA_RR_RL_FC_FL_FR;
        break;
    case AUD_DEC_TYPE_5_1:
        tPcmSetting.u1ChannelAssign = CA_RR_RL_FC_LFE_FL_FR;
        break;
    default:
        tPcmSetting.u1ChannelAssign = CA_FL_FR;
        LOG(LOG_FEATURE, TEXT("[AUD_MW][Err] Unknown channels of DVD LPCM setting.\n"));
        break;
    }

    tPcmSetting.u1IsBD = FALSE;
#if CONFIG_DRV_HDMI_RX
    vAudCodecSet_HDMIInPCMPara(&tPcmSetting);
#endif
}

static void AudSetDecWMAInfo(AUD_DEC_AUD_INFO_T * pv_set_info)
{
    if (!pv_set_info || !pv_set_info->u_fmt_spec.pt_wma_info)
    {
        LOG(LOG_FAIL, TEXT("[AUD_MW][WMA]Error of Parameter.\n"));
        VERIFY(0);
        return;
    }

    DspCodecSet_WmaInfo(pv_set_info);
    LOG(LOG_DECINFO, TEXT("[AUD_MW] Set WMA decoder information.\n"));
}

static void AudSetDecSACDInfo(AUD_DEC_AUD_INFO_T * pv_set_info)
{
    if (!pv_set_info || !pv_set_info->u_fmt_spec.pt_sacd_info)
    {
        LOG(LOG_FAIL, TEXT("[AUD_MW][SACD]Error of Parameter.\n"));
        VERIFY(0);
        return;
    }

    LOG(LOG_DECINFO, TEXT("[AUD_MW] Set SACD decoder information.\n"));
    vAudCodecSet_SACD_Input_Info(pv_set_info);
}

static void AudSetDecCOOKInfo(AUD_DEC_AUD_INFO_T * pv_set_info)
{
    AUD_DEC_RA_COOK_INFO_T*   ptCookInfo = pv_set_info->u_fmt_spec.pt_ra_cook_info;
    if (!pv_set_info || !pv_set_info->u_fmt_spec.pt_ra_cook_info)
    {
        LOG(LOG_FAIL, TEXT("[AUD_MW][COOK]Error of Parameter.\n"));
        VERIFY(0);
        return;
    }
    LOG(LOG_FEATURE, TEXT("[AUD_MW] Set COOK decoder information.\n"));
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_COOK_SAMPLE_PER_FRAME    = 0x%X.\n"), ptCookInfo->ui2_sample_per_frame);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_COOK_CHANNEL_NUM         = 0x%X.\n"), (pv_set_info->e_aud_type==AUD_DEC_TYPE_MONO)?1:2);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_COOK_INPUT_SAMPLING_RATE = 0x%X.\n"), ptCookInfo->u4_sample_rate);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_COOK_FRM_SZ_IN_BYTES     = 0x%X.\n"), ptCookInfo->ui4_frame_size_byte);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_COOK_REGION_NUM          = 0x%X.\n"), ptCookInfo->ui2_region_num);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_COOK_COUPLING_START_REGN = 0x%X.\n"), ptCookInfo->ui4_cpl_region_start);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_COOK_COUPLING_QBITS      = 0x%X.\n"), ptCookInfo->ui2_Q_bits_num);
    DspCodecSet_CookInfo(pv_set_info);
}

static void AudSetDecAPEInfo(AUD_DEC_AUD_INFO_T * pv_set_info)  //mtk70169 mark
{
    AUD_DEC_APE_INFO_T*   ptApeInfo = pv_set_info->u_fmt_spec.pt_ape_info;
    if (!pv_set_info || !pv_set_info->u_fmt_spec.pt_ape_info)
    {
        LOG(LOG_FAIL, TEXT("[AUD_MW][APE]Error of Parameter.\n"));
        VERIFY(0);
        return;
    }
    LOG(LOG_FEATURE, TEXT("[AUD_MW] Set APE decoder information.\n"));

    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_APE_FILE_VERSION = 0x%X.\n"), ptApeInfo->ui4_file_versoin);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_APE_COMPRESS_LEVEL = 0x%X.\n"), ptApeInfo->ui4_compress_level);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_APE_BLOCK_PER_FRAME = 0x%X.\n"), ptApeInfo->ui4_block_per_frame);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_APE_FINAL_FRAME_BLOCK = 0x%X.\n"), ptApeInfo->ui4_final_frame_block);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_APE_TOTAL_FRAME_NUM_LOW = 0x%X.\n"), ptApeInfo->ui4_total_frame_num);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_APE_TOTAL_FRAME_NUM_HIGH = 0x%X.\n"), ptApeInfo->ui4_total_frame_num);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_APE_BITS_PER_SAMPLE = 0x%X.\n"), ptApeInfo->ui4_bits_per_sample);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_APE_CHANNEL_NUM_1 = 0x%X.\n"), ptApeInfo->ui4_channel_num_1);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_APE_INPUT_SAMPLING_RATE = 0x%X.\n"), ptApeInfo->ui4_input_sampling_rate);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_APE_MUTE_BANK_NUMBERS = 0x%X.\n"), ptApeInfo->ui4_mute_bank_numbers);//mtk70105
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_APE_INVLID_BYTES = 0x%X.\n"), ptApeInfo->ui4_invalid_bytes);
    DspCodecSet_ApeInfo(pv_set_info);
}


void AudSetApeSeekInfo(u8 devId, APE_SEEKINFO_INFO_T * pApeSeekInfo)
{
    DspSetApeSeekInfo(devId, pApeSeekInfo);
}


static void AudSetDecFLACInfo(AUD_DEC_AUD_INFO_T * pv_set_info)
{
    AUD_DEC_FLAC_INFO_T*   ptFlacInfo = pv_set_info->u_fmt_spec.pt_flac_info;
    if (!pv_set_info || !pv_set_info->u_fmt_spec.pt_flac_info)
    {
        LOG(LOG_FAIL, TEXT("[AUD_MW][FLAC]Error of Parameter.\n"));
        VERIFY(0);
        return;
    }
    LOG(LOG_FEATURE, TEXT("[AUD_MW] Set FLAC decoder information.\n"));
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_MIN_BLK_SZ = 0x%X.\n"), ptFlacInfo->ui4_min_block_size);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_MAX_BLK_SZ = 0x%X.\n"), ptFlacInfo->ui4_max_block_size);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_MIN_FRM_SZ = 0x%X.\n"), ptFlacInfo->ui4_min_frame_size);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_MAX_FRM_SZ = 0x%X.\n"), ptFlacInfo->ui4_max_frame_size);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_SAMP_RATE = 0x%X.\n"), ptFlacInfo->ui4_sampling_rate);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_CH_NUM_1 = 0x%X.\n"), ptFlacInfo->ui2_channel_num_1);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_BPS_1 = 0x%X.\n"), ptFlacInfo->ui2_bits_per_sample_1);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_SMP_NUM_HI = 0x%X.\n"), ptFlacInfo->ui2_sample_num_high12);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_SMP_NUM_LO = 0x%X.\n"), ptFlacInfo->ui4_sample_num_low24);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_FRM_NUM_HI = 0x%X.\n"), ptFlacInfo->ui2_frame_num_high12);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_FRM_NUM_LO = 0x%X.\n"), ptFlacInfo->ui4_frame_num_low24);
    LOG(LOG_FEATURE, TEXT("[AUD_CODEC] set ADDR_RC2D_FLAC_STRM_END = 0x%X.\n"), ptFlacInfo->ui2_stream_end);

    DspCodecSet_FlacInfo(pv_set_info);
}

/***************************************************************************
* Function : AudSetDecInfo
* Description :
* Parameter :
* Return    : None
* Note      :
***************************************************************************/
void AudSetDecInfo(u8 u1DecId, AUD_DEC_AUD_INFO_T * pv_set_info)
{
    VERIFY(u1DecId <= TER_DEC);

    if ((pv_set_info->e_aud_fmt == AUD_DEC_FMT_PCM) &&
        (pv_set_info->e_aud_type >= AUD_DEC_TYPE_3_0))
    {
        vAdspSetMediaFlag(TRUE,1);
    }
    else
    {
        vAdspSetMediaFlag(FALSE,1);
    }

    if (u1DecId == PRI_DEC)
    {
        switch (pv_set_info->e_aud_fmt)
        {
        case AUD_DEC_FMT_PCM:
            AudSetDecPCMInfo(pv_set_info);
            break;
        case AUD_DEC_FMT_HDMI_IN_PCM:
            AudSetHDMIInPcmInfo(pv_set_info);
            break;
        case AUD_DEC_FMT_WMA:
            AudSetDecWMAInfo(pv_set_info);
            break;
        case AUD_DEC_FMT_SACD:
            AudSetDecSACDInfo(pv_set_info);
            break;
        case AUD_DEC_FMT_RA_COOK:
            AudSetDecCOOKInfo(pv_set_info);
            break;
        case AUD_DEC_FMT_APE:
            AudSetDecAPEInfo(pv_set_info);
            break;
        case AUD_DEC_FMT_FLAC:
            AudSetDecFLACInfo(pv_set_info);
            break;
        default:
            break;
        }
    }
    else if (u1DecId == SEC_DEC)
    {
        if(AUD_DEC_FMT_PCM == pv_set_info->e_aud_fmt)
        {
            LOG(LOG_CTRLF, _T("SEC_DEC No Need Set Decoder Info.\n"));
            //AudSetDecPCMInfo(pv_set_info);
        }
        else
        {
            LOG(LOG_CTRLF, _T("SEC_DEC Only support PCM.\n"));
        }
    }
    else if(u1DecId == TER_DEC)
    {
        LOG(LOG_CTRLF, _T("TER_DEC Set Decoder Info.\n"));
        if(AUD_DEC_FMT_PCM == pv_set_info->e_aud_fmt)
        {
            //AudSetDecPCMInfo(pv_set_info);
            AudSetDecSrcInfo(u1DecId, pv_set_info);
        }
        else
        {
            LOG(LOG_CTRLF, _T("TER_DEC Only support PCM.\n"));
        }
    }
}


#define LOG_AUD_DEC_INFO(rAudDecInfo)                                       \
    LOG(LOG_DATAF, TEXT("****fmt=%d,type=%d,sample=%d,bit=%d,pid=%d\r\n"),  \
        rAudDecInfo.e_aud_fmt,                                              \
        rAudDecInfo.e_aud_type,                                             \
        rAudDecInfo.ui4_sample_rate,                                        \
        rAudDecInfo.ui1_bit_depth,                                          \
        rAudDecInfo.ui2_pid)


#define LOG_PCM_INFO(prInfo)                                                   \
    LOG(LOG_DATAF, TEXT("****pcmfmt=%d, blockalign=%d,deemphasis=%d,dlna=%d\r\n"),  \
        prInfo->pcm_info.ePCM_Format,                                          \
        prInfo->pcm_info.u2BlockAlign,                                         \
        prInfo->pcm_info.b_de_emphasis,                                        \
        prInfo->pcm_info.b_dlna_exist)

#define LOG_RACOOK_INFO(prInfo)                                           \
    LOG(LOG_DATAF, TEXT("u4_sample_rate=%d, ui2_sample_per_frame=%d,           \
                         ui4_frame_size_byte=%d,ui2_region_num=%d,             \
                         ui4_cpl_region_start=%d, ui2_Q_bits_num=%d.\r\n"),    \
        prInfo->pt_ra_cook_info.u4_sample_rate,                   \
        prInfo->pt_ra_cook_info.ui2_sample_per_frame,             \
        prInfo->pt_ra_cook_info.ui4_frame_size_byte,              \
        prInfo->pt_ra_cook_info.ui2_region_num,                   \
        prInfo->pt_ra_cook_info.ui4_cpl_region_start,             \
        prInfo->pt_ra_cook_info.ui2_Q_bits_num)

#define LOG_WMA_INFO(prInfo)                                              \
    LOG(LOG_DATAF, TEXT("****blockalign=%d, encoption=%d, version=%d,          \
                         bytespersec=%d, packets=%d, packetsize=%d\r\n"),      \
        prInfo->wma_info.ui2_blockalign,                                  \
        prInfo->wma_info.ui2_enc_option,                                  \
        prInfo->wma_info.ui2_version,                                     \
        prInfo->wma_info.ui4_bytes_per_sec,                               \
        prInfo->wma_info.ui4_packet_count,                                \
        prInfo->wma_info.ui4_packet_size)


#define LOG_APE_INFO(prInfo)                                               \
    LOG(LOG_DATAF, TEXT("****version=%d,level=%d,blockPerFrame=%d,              \
                         finalFrameBlock=%d,frames=%d, bits=%d, channels=%d,    \
                         samplerate=%d,mutebank=%d, skip=%d\r\n"),              \
        prInfo->ape_info.ui4_file_versoin,                                 \
        prInfo->ape_info.ui4_compress_level,                               \
        prInfo->ape_info.ui4_block_per_frame,                              \
        prInfo->ape_info.ui4_final_frame_block,                            \
        prInfo->ape_info.ui4_total_frame_num,                              \
        prInfo->ape_info.ui4_bits_per_sample,                              \
        prInfo->ape_info.ui4_channel_num_1,                                \
        prInfo->ape_info.ui4_input_sampling_rate,                          \
        prInfo->ape_info.ui4_mute_bank_numbers,                            \
        prInfo->ape_info.ui4_invalid_bytes)

#define LOG_FLAC_INFO(prInfo)                                                  \
    LOG(LOG_DATAF, TEXT("****minBlock=%d,maxBlock=%d,minFrame=%d,maxFrame=%d,       \
                        samplerate=%d,channels=%d, bits=%d, sampleLow=0x%x,         \
                        sampleH=0x%x,frameL=0x%x, frameH=0x%x, streamend=%d\r\n"),  \
        prInfo->flac_info.ui4_min_block_size,                                  \
        prInfo->flac_info.ui4_max_block_size,                                  \
        prInfo->flac_info.ui4_min_frame_size,                                  \
        prInfo->flac_info.ui4_max_frame_size,                                  \
        prInfo->flac_info.ui4_sampling_rate,                                   \
        prInfo->flac_info.ui2_channel_num_1,                                   \
        prInfo->flac_info.ui2_bits_per_sample_1,                               \
        prInfo->flac_info.ui2_sample_num_high12,                               \
        prInfo->flac_info.ui4_sample_num_low24,                                \
        prInfo->flac_info.ui2_frame_num_high12,                                \
        prInfo->flac_info.ui4_frame_num_low24,                                 \
        prInfo->flac_info.ui2_stream_end)

bool AudSetMwCodecInfo(u8 u1DecId, AUD_INFO_T *prAudInfo)
{
    AUD_DEC_AUD_INFO_T rDecInfo = {0};
    if(prAudInfo == NULL)
    {
        LOG(LOG_CTRLF, TEXT("set codec info is null.\n"));
        return FALSE;
    }

    rDecInfo.e_aud_fmt       = prAudInfo->e_aud_fmt;
    rDecInfo.e_aud_type      = prAudInfo->e_aud_type;
    rDecInfo.ui4_sample_rate = prAudInfo->ui4_sample_rate;
    rDecInfo.ui4_data_rate   = prAudInfo->ui4_data_rate;
    rDecInfo.ui1_bit_depth   = prAudInfo->ui1_bit_depth;
    rDecInfo.ui2_pid         = prAudInfo->ui2_pid;

    LOG(LOG_CTRLF, TEXT("Audio format = 0x%x.\n"), prAudInfo->e_aud_fmt);

    if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_PCM)
    {
        rDecInfo.u_fmt_spec.pt_pcm_info = kzalloc(sizeof(AUD_DEC_PCM_INFO_T), GFP_KERNEL);
        if(NULL ==	rDecInfo.u_fmt_spec.pt_pcm_info)
        {
            LOG(LOG_FAIL, TEXT("PCM alloc memory fail\n"));
            return FALSE;
        }
        x_memcpy(rDecInfo.u_fmt_spec.pt_pcm_info, &prAudInfo->pcm_info, sizeof(prAudInfo->pcm_info));
        LOG_AUD_DEC_INFO(rDecInfo);
        LOG_PCM_INFO(prAudInfo);
    }
    else if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_HDMI_IN_PCM)
    {
        rDecInfo.u_fmt_spec.pt_pcm_info = kzalloc(sizeof(AUD_DEC_PCM_INFO_T), GFP_KERNEL);
        if(NULL ==	rDecInfo.u_fmt_spec.pt_pcm_info)
        {
            LOG(LOG_FAIL, TEXT("PCM alloc memory fail\n"));
            return FALSE;
        }
        x_memcpy(rDecInfo.u_fmt_spec.pt_pcm_info, &prAudInfo->pcm_info,
            sizeof(prAudInfo->pcm_info));

        LOG_AUD_DEC_INFO(rDecInfo);
        LOG_PCM_INFO(prAudInfo);
    }
    else if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_RA_COOK)
    {
        rDecInfo.u_fmt_spec.pt_ra_cook_info = kzalloc(sizeof(AUD_DEC_RA_COOK_INFO_T), GFP_KERNEL);
        if(NULL ==	rDecInfo.u_fmt_spec.pt_ra_cook_info)
        {
            LOG(LOG_FAIL, TEXT("Cook alloc memory fail.\r\n"));
            return FALSE;
        }
        x_memcpy(rDecInfo.u_fmt_spec.pt_ra_cook_info, &prAudInfo->pt_ra_cook_info, sizeof(prAudInfo->pt_ra_cook_info));
        LOG_RACOOK_INFO(prAudInfo);
    }
    else if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_WMA)
    {
        rDecInfo.u_fmt_spec.pt_wma_info = kzalloc(sizeof(AUD_DEC_WMA_INFO_T), GFP_KERNEL);
        if(NULL ==  rDecInfo.u_fmt_spec.pt_wma_info)
        {
            LOG(LOG_FAIL, TEXT("WMA alloc memory fail\r\n"));
            return FALSE;
        }
        x_memcpy(rDecInfo.u_fmt_spec.pt_wma_info, &prAudInfo->wma_info, sizeof(prAudInfo->wma_info));
        LOG_AUD_DEC_INFO(rDecInfo);
        LOG_WMA_INFO(prAudInfo);
    }
    else if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_APE)
    {
        rDecInfo.u_fmt_spec.pt_ape_info = kzalloc(sizeof(AUD_DEC_APE_INFO_T), GFP_KERNEL);
        if(NULL ==	rDecInfo.u_fmt_spec.pt_ape_info)
        {
            LOG(LOG_FAIL, TEXT("APE alloc memory fail.\r\n"));
            return FALSE;
        }

        x_memcpy(rDecInfo.u_fmt_spec.pt_ape_info, &prAudInfo->ape_info, sizeof(prAudInfo->ape_info));
        LOG_AUD_DEC_INFO(rDecInfo);
        LOG_APE_INFO(prAudInfo);
    }
    else if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_FLAC)
    {
        rDecInfo.u_fmt_spec.pt_flac_info = kzalloc(sizeof(AUD_DEC_FLAC_INFO_T), GFP_KERNEL);
        if(NULL == rDecInfo.u_fmt_spec.pt_flac_info)
        {
            LOG(LOG_DATAF, TEXT("FLAC alloc memory fail.\r\n"));
            return FALSE;
        }
        x_memcpy(rDecInfo.u_fmt_spec.pt_flac_info, &prAudInfo->flac_info, sizeof(prAudInfo->flac_info));
        LOG_AUD_DEC_INFO(rDecInfo);
        LOG_FLAC_INFO(prAudInfo);
    }

    AudSetDecInfo(u1DecId, (AUD_DEC_AUD_INFO_T *)&rDecInfo);
    AudBackupDecInfo(u1DecId,(AUD_DRV_AUD_INFO_T *)(&rDecInfo));
    if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_PCM) {
        kfree((void *)(rDecInfo.u_fmt_spec.pt_pcm_info));
    }
    else if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_HDMI_IN_PCM) {
				kfree((void *)(rDecInfo.u_fmt_spec.pt_pcm_info));
    }
    else if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_RA_COOK) {
        kfree((void *)(rDecInfo.u_fmt_spec.pt_ra_cook_info));
    }
    else if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_WMA){
				kfree((void*)(rDecInfo.u_fmt_spec.pt_wma_info));
    }
    else if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_APE){
        kfree((void *)(rDecInfo.u_fmt_spec.pt_ape_info));
    }
    else if (prAudInfo->e_aud_fmt == AUD_DEC_FMT_FLAC){
        kfree((void *)(rDecInfo.u_fmt_spec.pt_flac_info));
    }

    return TRUE;
}



s32 AudSetDecPlaySpeed(u8 u1DecId, AUD_DEC_PB_SPEED_TYPE_T rSpeed)
{
    s32 i4Ret = i4AudSetPlaySpeed(u1DecId, rSpeed);

    vSendADSPCmd(UOP_DSP_SPEED);

    return i4Ret;
}

s32 AudSetDspAsrcBypass(u8 u1DecId, bool fgVal)
{
    LOG(LOG_CTRLF, _T("SetDspAsrcBypass Dec %d, val %d. \r\n"), u1DecId, fgVal);

    if(1==DspSetAsrcBypass(u1DecId, fgVal))
    {
         LOG(LOG_CTRLF, _T("ByPass Mode is same. \r\n"));
    }
    return TRUE;
}


bool AudAllocDecResource(AUD_DRV_CONTEXT *prContext, u8 u1Out, AUD_OUT_MEDIA_TYPE_T eType)
{
    bool fgRet = FALSE;
    if(AUD_FRONT == u1Out)
    {
        if(fgAdspSetFrontAoutMediaType(eType))
        {
            AudSetDecContext(eType, prContext, AUD_FRONT);
        }
        else
        {
             LOG(LOG_CTRLF,_T("alloc front audio res error.\r\n"));
             fgRet = FALSE;
        }
    }
    else if(AUD_REAR == u1Out)
    {
        if(fgAdspSetRearAoutMediaType(eType))
        {
            AudSetDecContext(eType, prContext, AUD_REAR);
        }
        else
        {
            LOG(LOG_CTRLF,_T("alloc rear audio res error.\r\n"));
            fgRet = FALSE;
        }
    }
    return (fgRet);
}

void AudReleaseDecResource(AUD_DRV_CONTEXT *prContext)
{
    LOG(LOG_CTRLF, TEXT("AudReleaseDecResource.\r\n"));
    vWriteDspShmWORD(W_MEDIA_TYPE, 0);
	prContext->u1Output= AUD_OUT_MAX;
    prContext->fgEnPlay = FALSE;
	prContext->u1DecId = MAX_AUDDRV_NUM;
	prContext->fgPlaying= FALSE;
}



bool AudSetDecPlayBackInfo(AUD_DRV_CONTEXT *prContext, AUD_DEC_AUDIO_PB_INFO_T* prPbInfo)
{
    bool fgRet = FALSE;
    AUD_DRV_FMT_INFO_T eDrvInfo = {0};

    //1.set format to load code
    eDrvInfo.e_fmt = prPbInfo->eAudDecFmt;
    fgRet = AudSetFormat(prContext->u1DecId, &eDrvInfo);

	//2.set audio decoder information
    if(TRUE == fgRet && prPbInfo->prInfo!=NULL)
    {
        fgRet = AudSetMwCodecInfo(prContext->u1DecId, prPbInfo->prInfo);
    }

	//3. set playback speed
    if(TRUE == fgRet)
    {
        AudSetDecPlaySpeed(prContext->u1DecId, prPbInfo->eSpeed);
    }
    return (fgRet);
}


void AudSetMasterVolPolicy(AUD_VOL_POLICY_T eType, u32 u4VolGain)
{
    LOG(LOG_CTRLF, TEXT("set volume: type %d, volume = 0x%x, master volume = 0x%x, PolicyStatus %d.\r\n"),
		eType, u4VolGain, g_rFrnVolGain.u4VolMaster, g_fgPolicyStatus);

    switch(eType)
    {
    case AUD_VOL_NORMAL:
        if(FALSE == g_fgPolicyStatus)
        {
            if(u4VolGain != g_rFrnVolGain.u4VolMaster)
            {
                g_rFrnVolGain.u4VolMaster = u4VolGain;
                vAdspMasterVolumeGain(u4VolGain);
            }
        }
        else
        {
            g_rFrnVolGain.u4VolMaster = u4VolGain;
        }
        break;
    case AUD_VOL_SET_POLICY:
        g_fgPolicyStatus = TRUE;
        vAdspMasterVolumeGain(u4VolGain);
        break;

    case AUD_VOL_RESET_POLICY:
        g_fgPolicyStatus = FALSE;
        vAdspMasterVolumeGain(g_rFrnVolGain.u4VolMaster);
        break;

    default:
        LOG(LOG_CTRLF, _T("AudSetVolPolicy unsupport type.\n"));
        break;
    }
}


