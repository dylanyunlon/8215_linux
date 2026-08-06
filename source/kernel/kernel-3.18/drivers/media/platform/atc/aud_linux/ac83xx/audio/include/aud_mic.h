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


#ifndef _AUD_MIC_H_
#define _AUD_MIC_H_

#include "x_typedef.h"
#include <media/atc/drv_av_d.h>


#define  MIC_MIXING_TO_L_R           ((1 << 1) << 8)
#define  MIC_MIXING_TO_LS_RS         ((1 << 2) << 8)
#define  MIC_MIXING_TO_C             ((1 << 3) << 8)
#define  MIC_MIXING_TO_EXT_CH1_CH2   ((1 << 4) << 8)
#define  MIC_MIXING_TO_DMX_CH1_CH2   ((1 << 5) << 8)
#define  MIC_MIXING_TO_CH11_CH12     ((1 << 6) << 8)
#define  MIC_MIXING_TO_SUB           ((1 << 7) << 8)
#define  MIC_DEFAULT_MIXING   (MIC_MIXING_TO_L_R)

enum MicChannelMixType {
    MIC_CHANNEL_MIX_AOUT1,
    MIC_CHANNEL_MIX_AOUT2
};

#define MIC_DEFAULT_MUTE_BANK (5 << 8)

#define MIC_DEFAULT_ECHO_VOLUME  (0x200000)
#define MIC_DEFAULT_ECHO_FABGAIN (0x200000)

#define MIC_DEFAULT_LEVEL_UP_ORDER   (4 << 8)
#define MIC_DEFAULT_LEVEL_DOWN_ORDER (4 << 8)

#define MIC_DEFAULT_BUFFER_ADDRESS   (0xB80)


/* SET methods */
void vAudSetMicInit(const struct AudMicInitPara *prMicInitPara);
void vAudSetMicUnInit(void);
void vAudSetMicOn(void);
void vAudSetMicOff(void);
void vAudSetMicVolume1(u32 u4Volume1);
void vAudSetMicVolume2(u32 u4Volume2);
void vAudSetMicThreshold(u32 u4MuteThreshold, u32 u4UnmuteThreshold);
void vAudSetMicMuteBank(u32 u4MuteBank);
void vAudSetMicEchoVolume(u32 u4EchoVolume);
void vAudSetMicEchoFBGain(u32 u4EchoFBGain);
void vAudSetMicEchoDelay(AUDIO_SAMPLING_T eSourceFS, u32 u4EchoDelayMS);
void vAudSetMicChannelMix(enum MicChannelMixType eType, u32 u4ChannelMix);
void vAudSetMicLevelUpOrder(u32 u4LevelUpOrder);
void vAudSetMicLevelDownOrder(u32 u4LevelDownOrder);
void vAudSetMic_OnSourceFSChange(AUDIO_SAMPLING_T eSourceFS);
void vAudSetMic_OnAoutOnDone(void);


/* GET methods */
bool   fgAudGetMicIsOn(void);
u32 u4AudGetMicVolume1(void);
u32 u4AudGetMicVolume2(void);
u32 u4AudGetMicThresholdMute(void);
u32 u4AudGetMicThresholdUnmute(void);
u32 u4AudGetMicMuteBank(void);
u32 u4AudGetMicEchoVolume(void);
u32 u4AudGetMicEchoFBGain(void);
u32 u4AudGetMicEchoDelayValue(void);
u32 u4AudGetMicChannelMix1(void);
u32 u4AudGetMicChannelMix2(void);
u32 u4AudGetMicLevelUpOrder(void);
u32 u4AudGetMicLevelDownOrder(void);
typedef u32 MicPrintWhat;
#define MIC_PRINT_ONOFF         (1 <<  0)
#define MIC_PRINT_VOLUME1       (1 <<  1)
#define MIC_PRINT_VOLUME2       (1 <<  2)
#define MIC_PRINT_THRESHHOLD    (1 <<  3)
#define MIC_PRINT_MUTE_BANK     (1 <<  4)
#define MIC_PRINT_ECHO_VOLUME   (1 <<  5)
#define MIC_PRINT_ECHO_FBGAIN   (1 <<  6)
#define MIC_PRINT_ECHO_DELAY    (1 <<  7)
#define MIC_PRINT_CHANNEL_MIX   (1 <<  8)
#define MIC_PRINT_LEVEL_ORDER   (1 <<  9)
#define MIC_PRINT_INPUT_LEVEL   (1 << 10)
#define MIC_PRINT_ALL (0xFFFFFFFF)
void vAudPrintMicStatus(MicPrintWhat u4PrintWhat);


//Audio Path
void vAudSetPath(enum AudPath ePath);
enum AudPath eAudGetPath(void);


//Key Control
s32 i4AudSetKeyShift(u32 u4KeyShiftValue);
u32 u4AudGetKeyShift(void);
//KeyShift Traval (for testing)
enum KeyShiftTraverseMode {
    KEY_SHIFT_TRAVERSE_MODE_ELEVATOR,
    KEY_SHIFT_TRAVERSE_MODE_REPEAT,
    KEY_SHIFT_TRAVERSE_MODE_RANDOM
};
void vSetKeyShiftTraverseMode(enum KeyShiftTraverseMode eKeyShiftTraverseMode);
void vResetKeyShiftTraverse(void);
u32 u4GetNextKeyShiftValue(void);


#endif

