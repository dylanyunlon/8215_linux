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

#ifndef _AUD_DRV_CONFIG_H
#define _AUD_DRV_CONFIG_H

//for flexible audio out delay
#define AUD_AOUT_MAX_CH_NUM                         10      // 384M aout default 10 channel
#define AUD_AOUT2_MAX_CH_NUM                        8       // 384M aout default 10 channel
#define AUD_AOUT_CH1_SADR                           0x2000
#define AUD_AOUT2_CH1_SADR                          0x1000

//for aout bypass 2ch downmix enable

// for new aout reinit flow
#define AUD_DRV_AOUT_REINIT_SUPPORT                 0


// (aud_se_v2)
#define CONFIG_AUD_SE_V2_EN                         1

#define CONFIG_DRV_SUPPORT_DVD_AUDIO                0

#define AUD_DRV_DECODING_CHANNEL_AVD                1

#define CONFIG_DRV_SPDIF_RAW_SUPPORT                1

#define AUD_DSPC_SUPPORT                            0

#define NEW_STEP_FLOW
// don't need to check dsp's status, just give uop to dsp
// Audio driver can not ignor skip command

#define NEW_FRAME_ACCURATE_CMD
//Syncctrl /AUD driver can give frame accurate command any time and
//ADec do not clean frame accurate command when stop untile frame accurate done or driver disconnect
//Use case: A-B repeat and change audio, frame accurate command do not be send again

#define CONFIG_AUD_POWER_MANAGEMENT_SUPPORT         1
#define CONFIG_AUD_PM_SIMPLE_VERSION                1

#define NEW_DIVERSITY_SCHEME                        0
#define CONFIG_DRV_AUDIO_EXTERNAL_POST_PROC_SUPPORT 0
#define CONFIG_DRV_AUDIO_EXTERNAL_POST_PROC_ON_ARM1 0
#define AUD_DRV_LIP_SYNC_SUPPORT                    0

#define CONFIG_AUD_ADSP_ERR_RECOVER_EN              1

#define CONFIG_AUD_DECONLY_EN                       1

#endif  // _AUD_DRV_CONFIG_H
