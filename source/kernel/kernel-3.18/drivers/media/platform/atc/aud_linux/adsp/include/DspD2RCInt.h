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



#ifndef _H_D2RCINT
#define  _H_D2RCINT

#include "aud_drv_config.h"
/*
 * Interrupts from Dsps to Risc with the format of 0xaabbccdd, where
 * 0xdd is RIntAddr
 * 0xaabbcc is the RInt_SD
*/

//========================================================================
// DSP A
//========================================================================
#define INT_D2RC_PTS_1ST_REFRESH                0x00F0

// Step INT
#define INT_D2RC_MIXER_STEP                     0x00F6
#define D2RC_STEP_CMD_OK                        0x0000
#define D2RC_STEP_DONE                          0x0100
#define D2RC_SEC_STEP_CMD_OK                    0x0200
#define D2RC_SEC_STEP_DONE                      0x0300

#define D2RC_STEPTOEND_CMD_OK                   0x0400
#define D2RC_STEP_CANCEL_OK                     0x0500
#define D2RC_SEC_STEPTOEND_CMD_OK               0x0600
#define D2RC_SEC_STEP_CANCEL_OK                 0x0700

// Frame accurate INT
#define INT_D2RC_MIXER_PTS_ACCURATE             0xF7
#define D2RC_PTS_BEGIN_DONE                     0x000
#define D2RC_PTS_END_DONE                       0x100
#define D2RC_SEC_PTS_BEGIN_DONE                 0x200
#define D2RC_SEC_PTS_END_DONE                   0x300
#define D2RC_PAUSE_PTS_BEGIN_DONE               0x400
#define D2RC_SEC_PAUSE_PTS_BEGIN_DONE           0x500
#define D2RC_STEP_PTS_END_DONE                  0x600
#define D2RC_SEC_STEP_PTS_END_DONE              0x700

#define INT_D2RC_AOUT2_STATUS                   0x00F8
#define AOUT2_CONTROL_AOUT2_ON                  0x000
#define AOUT2_CONTROL_AOUT2_OFF                 0x100

#define INT_D2RC_MIXER_REAL_DISCONNECT_OK       0x00F9
#define INT_D2RC_MIXER_REAL_CONNECT_OK          0x00FA         // -- Water (AUD_RIPPING)

#define INT_D2RC_DISCONNECT_CMD                 0x00FB
#define INT_D2RC_CONNECT_CMD                    0x00FC
    #define D2RC_PRIMARY_DECODER                0x100
    #define D2RC_SECONDARY_DECODER              0x200
    #define D2RC_FOURTH_DECODER                 0x400
    #define D2RC_FIFTH_DECODER                  0x500


#define INT_D2RC_DSP_AOUT_NOTIFY                0x00FD
#define FIRST_AOUT_NOTIFY                       0x200
#define LAST_AOUT_NOTIFY                        0x300
#define DEC2_FIRST_AOUT_NOTIFY                  0x400
#define DEC2_LAST_AOUT_NOTIFY                   0x500
#define DEC3_FIRST_AOUT_NOTIFY                  0x600
#define DEC3_LAST_AOUT_NOTIFY                   0x700


#define INT_D2RC_AOUT_STATUS                    0x00FE
#define AOUT_CONTROL_AOUT_ON                    0x000
#define AOUT_CONTROL_AOUT_OFF                   0x100

#define INT_D2RC_REENCODER_STATUS               0x00FF
#define REENC_STOP_OK                           0x000
#define REENC_START_CMD_OK                      0x100
#define REENC_SEND_BITSTREAM                    0x200
#define REENC_STOP_CMD_OK                       0x300
#define ENCODER_FLUSH_CMD_OK                    0x400          // -- Water (AUD_RIPPING)

// (aud_8550_emu_migration)
#define INT_D2RC_BIM_TSTA                       0x00D1

//gpsmix ---add by fei
#define INT_D2RC_EXTMIX_STATUS              0x00DF
#define EXTMIX_PLAY_OK                      0x00100
#define EXTMIX_STOP_OK                      0x00200
#define EXTMIX_PAUSE_OK                     0x00300
#define EXTMIX_PLAY_GET                     0x00400
#define EXTMIX_STOP_GET                     0x00500
#define EXTMIX_PAUSE_GET                    0x00600
#define EXTMIX_DATA_CONSUMED                0x00700

#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
#define INT_D2RC_DSPA_STATUS                0x00DE
#define DSPA_WAKE_UP_OK                     0x00100
#endif

#define INT_DVP_MIX_STATUS                     0x00DD

#define INT_AWB_STATUS                      0x00D8

//========================================================================
// DSP B
//========================================================================

//Decoder
#define INT_D2RC_FLOW_CONTROL                   0x00FF
#define INT_D2RC_FLOW_CONTROL2                  0x00FE
#define INT_D2RC_FLOW_CONTROL_DEC3              0x00FD
#define INT_D2RC_FLOW_CONTROL_DEC4              0x00FB
#define INT_D2RC_FLOW_CONTROL_DEC5              0x00FA
    #define D2RC_FLOW_CONTROL_STOP_OK           0x0000
    #define D2RC_FLOW_CONTROL_SAMPLING_RATE     0x0100
    #define D2RC_FLOW_CONTROL_END_OF_STREAM     0x0200
    #define D2RC_FLOW_CONTROL_PLAY_GOT          0x0300
    #define D2RC_FLOW_CONTROL_DEC_READY         0x0400
    #define D2RC_FLOW_CONTROL_AUDIO_UNDERRUN    0x0500

// In band command
#define INT_D2RC_PRIMARY_INBAND_CMD             0x0088 // temperal solution
#define INT_D2RC_SECONDARY_INBAND_CMD           0x00FC

//De-emphasis notify
#define INT_D2RC_DEEMPHASIS_NOITFY              0x00F7
#define DEEMPH_ENABLE                           0x01
#define DEEMPH_DISABLE                          0x00

//HDCD
#define INT_D2RC_HDCD_TRK_STM_CHG               0x0003
#define D2RC_STM_HDCD_OFF                       0x01
#define D2RC_STM_HDCD_ON                        0x02

// MP3
#define INT_D2RC_MP3_TYPE                       0x0005

//VORBIS Table
#define INT_D2RC_VORBIS_CODEBOOK                0x0004

//Secondary metadata update interrupt
#define INT_D2RC_MIXING_METADATA_UPDATE         0x0007

// DspLogDump
#define INT_R2RC_FLUSH_DSPLOG                   0xD0

// (aud_8550_emu_migration)
#define INT_D2RC_BIM_TST                        0xD1

#if CONFIG_AUD_POWER_MANAGEMENT_SUPPORT
#define INT_D2RC_DSPB_STATUS                0x00DD
#define DSPB_WAKE_UP_OK                     0x0100
#endif

#define INT_D2RC_POST_REINIT_STATUS            0x00DC
#define PR_FADE_OUT_OK                        0x0000
#define PR_FADE_IN_OK                        0x0100


//RC2D part
#define INT_RC2D_SPEED                      0x0011
#define INT_RC2D_PROCESSING_MODE            0x0025

//RISC Read DSP Memory
#define INT_RC2D_READ_DSP_MEMORY            0xF7  //DSP_NEW_AVSYNC_SUPPORT



#endif

