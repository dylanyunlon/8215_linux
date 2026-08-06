/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef _ARM2_PCM_PLAY_IF_H
#define _ARM2_PCM_PLAY_IF_H

#include "aud_comm.h"

#define AUD_DEF_LEFT_VOLUME             0x10000         //0dB, other value refer to GainMap 
#define AUD_DEF_RIGHT_VOLUME            0x10000

#if defined (DAC_TYPE_PWM)
#define AUD_DEF_DAC_TYPE                AUD_DAC_PWM
#else
#define AUD_DEF_DAC_TYPE                AUD_DAC_EXT
#endif

                                                        //BIT: 8~11, 12~15, 16~19, 20~23
#define AUD_DEF_CH_CFG0                 0xFF2100        //      FL     FR     C     CH7
#define AUD_DEF_CH_CFG1                 0xFFFF00        //      RL     RR    LFE    CH8
#define AUD_DEF_CH_CFG1_2TO4            0xFF2100        //      
#define AUD_DEF_CH_CFG1_4TO4            0xFF4300        //      
#define AUD_DEF_CH_NUM                  2 

#define AUD_TWO_SPEAKERS               2
#define AUD_FOUR_SPEAKERS              4

typedef struct
{
    UINT32 u4SampleRate; 
    UINT32 u4BitsPerSamples; 
    UINT32 u4Channels;

    LPBYTE lpData; 
    UINT32 u4Len; 
    UINT32 u4Loops;   
}ARM2PCM_FMT, *PARM2PCM_FMT;


extern UINT32 ARM2PCM_Start(PARM2PCM_FMT prFmt);
extern UINT32 ARM2PCM_Stop();

extern VOID ARM2PCM_Init(UINT32 u4SpeakerNum);
extern UINT32 ARM2PCM_Open();
extern UINT32 ARM2PCM_Close();


#endif /* _ARM2_PCM_PLAY_IF_H */
