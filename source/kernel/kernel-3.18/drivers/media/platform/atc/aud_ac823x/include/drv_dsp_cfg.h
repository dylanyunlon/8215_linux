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

#ifndef DRV_DSP_CFG_H
#define DRV_DSP_CFG_H

#include <media/atc/drv_aud.h>

typedef enum
{
    AUD_CH_FRONT_LEFT = 0,
    AUD_CH_FRONT_RIGHT,
    AUD_CH_REAR_LEFT,
    AUD_CH_REAR_RIGHT,
    AUD_CH_CENTER,
    AUD_CH_SUB_WOOFER,
    AUD_CH_BACK_LEFT,
    AUD_CH_BACK_RIGHT,
    AUD_CH9,
    AUD_CH10,
    AUD_CH_ALL
}   AUD_CH_T;

extern void vAdspAC3DRCRange(u8 uDRCLevel,u8 uDecIndex);
extern void vAdspSetSpeakerConfig(AUD_DEC_SPEAKER_LAYOUT_T rSpeakerLayout);

extern void vAdspSetMediaFlag(bool fgFlag,u8 uBit);
//extern void vAdspSettingIEC_HDMIPCMCH(void);
//extern void vAdspSettingHDMIPROCMode(void);

extern u8 u1AsvDspAAout2Off(void);
extern u8 u1AsvDspAAoutOff(void);

// UI_default
extern void vAdspMasterVolume(u8 u1Volume);
extern void vAdspRearChVolGainCtrl(u32 u4VolGainValue);

extern bool fgAdspIECConfig(u8 ucDecId, AUD_IEC_CFG_T eIecCfg, bool fgEnable);

extern void vAdspSetModBManagementInfo(AUD_DEC_MODULE_BMANAGEMENT_CHANNEL_INFO_T eModBManagementInfo);
//extern void vAdspSetDiversityInfo(AUD_DEC_DIV_TYPE_T eDiversityType, u8 u1_Setting);
extern void vAdspSetFeatureInfo(AUD_DEC_FEATURE_INFO_T eModFeatureInfo);
extern void vAdspMasterVolumeGain(u32 u4Volume);

#endif /* DRV_DSP_CFG_H */
