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
#ifndef _ARM2_PCM_PLAY_H
#define _ARM2_PCM_PLAY_H

#include "arm2pcmplay_if.h"
#include "aud_pinmux.h"
#include "aud_aout.h"
#include "types.h"

//===================================================================================================//

#define RESERVE_BUFFER                  (AOUT_INTR_TIME * 48 * 3) 

#define TAKE_AUD_POWER_ON_SEMAPHORE()    BIM_GETHWSemaphore(HWSMPHE_AUD_POWER_ON, 10)
#define RELEASE_AUD_POWER_ON_SEMAPHORE() BIM_ReleaseHWSemaphore(HWSMPHE_AUD_POWER_ON)

#define TAKE_GPS_AOUT_SEMAPHORE()        AUDLOG_INFO((T("ARM2 Take Aout Semaphone!!!\r\n")));\
                                         BIM_GETHWSemaphore(HWSMPHE_GPSAOUT, 0)
#define RELEASE_GPS_AOUT_SEMAPHORE()     AUDLOG_INFO((T("ARM2 Release Aout Semaphone!!!\r\n")));\
                                         BIM_ReleaseHWSemaphore(HWSMPHE_GPSAOUT)

//===================================================================================================//

typedef enum
{
    STR_OPEN = 0,
    STR_RUN,
    STR_STOP,
    STR_CLOSE
}STR_STATE;


typedef struct
{
    STR_STATE eState;
    BOOL fgInitialized;
    BOOL fgPowerOnByArm11;

    UINT32 u4BW;
    UINT32 u4Chn;
    LPBYTE lpCurrData;
    LPBYTE lpDataEnd;
    LPBYTE lpDataStart;
    UINT32 u4LoopCnt;

    UINT32 u4AOutLen;
    UINT32 u4OutWP;
    UINT32 u4FxGain[2];

    BOOL fgDoFadeOut;
    UINT32 u4SpeakerNum;

}PCM_PLAY_T, *PPCM_PLAY_T;

//===================================================================================================//


#endif /* _ARM2_PCM_PLAY_H */