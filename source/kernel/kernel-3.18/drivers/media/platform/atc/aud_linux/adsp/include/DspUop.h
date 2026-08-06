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




/*
 * DspUop command format is in the format of 0xaabbccddeeffgghh, where
 * 0xhh is the command group id.
 * If it is a control message and doesn't belong to the interrupt group, then
 * 0xff is the playback id, 0xgg is the command type, and 0xee is reserved as zero
 * Or if it belongs to the interrupt group, then
 * 0xdd is the Interrupt Addr and
 * 0xaabbcc and 0xeeffgg are RInt_SD and RINT_LD each
 */

#define UOP_DSP                           0x00

#define ADSPTASK_MSG_COMMAND              (UOP_DSP + 0x00)
#define ADSPTASK_MSG_POWER_ON             (ADSPTASK_MSG_COMMAND+(0x01<<8))
#define ADSPTASK_MSG_POWER_OFF            (ADSPTASK_MSG_COMMAND+(0x02<<8))

/************************************************************************/
// Volume UOP
/************************************************************************/
//Volume of first decoder
#define DSP_UOPID1                        (UOP_DSP + 0x01)
#define UOP_DSP_MASTER_VOLUME             (DSP_UOPID1 + (0x01 << 8))
#define UOP_DSP_TRIM_C                    (DSP_UOPID1 + (0x02 << 8))
#define UOP_DSP_TRIM_L                    (DSP_UOPID1 + (0x03 << 8))
#define UOP_DSP_TRIM_R                    (DSP_UOPID1 + (0x04 << 8))
#define UOP_DSP_TRIM_LS                   (DSP_UOPID1 + (0x05 << 8))
#define UOP_DSP_TRIM_RS                   (DSP_UOPID1 + (0x06 << 8))
#define UOP_DSP_TRIM_CH7                  (DSP_UOPID1 + (0x07 << 8))
#define UOP_DSP_TRIM_CH8                  (DSP_UOPID1 + (0x08 << 8))
#define UOP_DSP_TRIM_SUBWOOFER            (DSP_UOPID1 + (0x09 << 8))
#define UOP_DSP_TRIM_LFE                  (DSP_UOPID1 + (0x0A << 8))
#define UOP_DSP_DIALOGUE_GAIN             (DSP_UOPID1 + (0x0B << 8))
#define UOP_DSP_RAW_MUTE                  (DSP_UOPID1 + (0x0C << 8))
#define UOP_DSP_RAW_UNMUTE                (DSP_UOPID1 + (0x0D << 8))
#define UOP_DSP_RAW_MUTE_HDMI             (DSP_UOPID1 + (0x0E << 8))
#define UOP_DSP_RAW_UNMUTE_HDMI           (DSP_UOPID1 + (0x0F << 8))

//Volume of second decoder
#define UOP_DSP_MASTER_VOLUME_DEC2        (DSP_UOPID1 + (0x11 << 8))
#define UOP_DSP_TRIM_L_DEC2               (DSP_UOPID1 + (0x12 << 8))
#define UOP_DSP_TRIM_R_DEC2               (DSP_UOPID1 + (0x13 << 8))
#define UOP_DSP_TRIM_LFE_DEC2             (DSP_UOPID1 + (0x14 << 8))
#define UOP_DSP_DIALOGUE_GAIN_DEC2        (DSP_UOPID1 + (0x15 << 8))
#define UOP_DSP_RAW_MUTE_DEC2             (DSP_UOPID1 + (0x16 << 8))
#define UOP_DSP_RAW_UNMUTE_DEC2           (DSP_UOPID1 + (0x17 << 8))
#define UOP_DSP_REAR_MASTER_VOLUME        (DSP_UOPID1 + (0x18 << 8))

/************************************************************************/
// Mixsound UOP
/************************************************************************/
#define DSP_UOPID2                        (UOP_DSP + 0x02)
#define UOP_DSP_BDJ_MIXING_P1_P2_CHANGE   (DSP_UOPID2 + (0x02 << 8))

/************************************************************************/
// Flow Control UOP
/************************************************************************/
#define DSP_UOPID3                        (UOP_DSP + 0x03)
#define DSP_PLAY                          (DSP_UOPID3 + (0x01 << 8))
#define DSP_STOP                          (DSP_UOPID3 + (0x02 << 8))
#define DSP_FLUSH                         (DSP_UOPID3 + (0x03 << 8))
#define DSP_PAUSE                         (DSP_UOPID3 + (0x04 << 8))
#define DSP_RESUME                        (DSP_UOPID3 + (0x05 << 8))
#define DSP_RESET                         (DSP_UOPID3 + (0x06 << 8))
#define DSP_WAKEUP                        (DSP_UOPID3 + (0x07 << 8))
#define DSP_GETBITRATE                    (DSP_UOPID3 + (0x08 << 8))
#define DSP_GETMPEG_TYPE                  (DSP_UOPID3 + (0x09 << 8))
#define DSP_RISC_SKIP_DONE                (DSP_UOPID3 + (0x0A << 8))
#define DSP_INBAND_CMD_DONE               (DSP_UOPID3 + (0x0B << 8))
#define DSP_DEC2_METADATA_ACK             (DSP_UOPID3 + (0x0C << 8))
#define DSP_DEC_REINIT                    (DSP_UOPID3 + (0x0D << 8))

/************************************************************************/
// Bass Management UOP
/************************************************************************/
// Channel Configuration
#define DSP_UOPID4                        (UOP_DSP + 0x04)
#define UOP_DSP_CONFIG_SPEAKER            (DSP_UOPID4 + (0x01 << 8))
#define UOP_DSP_CONFIG_DELAY_C            (DSP_UOPID4 + (0x02 << 8))
#define UOP_DSP_CONFIG_DELAY_L            (DSP_UOPID4 + (0x03 << 8))
#define UOP_DSP_CONFIG_DELAY_R            (DSP_UOPID4 + (0x04 << 8))
#define UOP_DSP_CONFIG_DELAY_LS           (DSP_UOPID4 + (0x05 << 8))
#define UOP_DSP_CONFIG_DELAY_RS           (DSP_UOPID4 + (0x06 << 8))
#define UOP_DSP_CONFIG_DELAY_CH7          (DSP_UOPID4 + (0x07 << 8))
#define UOP_DSP_CONFIG_DELAY_CH8          (DSP_UOPID4 + (0x08 << 8))
#define UOP_DSP_CONFIG_DELAY_SUBWOOFER    (DSP_UOPID4 + (0x09 << 8))
#define UOP_DSP_CONFIG_DELAY_CH9          (DSP_UOPID4 + (0x0A << 8))
#define UOP_DSP_CONFIG_DELAY_CH10         (DSP_UOPID4 + (0x0B << 8))

// Channel configuration for second decoder
#define UOP_DSP_CONFIG_SPEAKER_DEC2       (DSP_UOPID4 + (0x11 << 8))
#define UOP_DSP_CONFIG_DELAY_C_DEC2       (DSP_UOPID4 + (0x12 << 8))
#define UOP_DSP_CONFIG_DELAY_L_DEC2       (DSP_UOPID4 + (0x13 << 8))
#define UOP_DSP_CONFIG_DELAY_R_DEC2       (DSP_UOPID4 + (0x14 << 8))

/************************************************************************/
// IEC, PTS, STC and Speed UOP
/************************************************************************/
#define DSP_UOPID5                        (UOP_DSP + 0x05)
// first decoder
#define UOP_DSP_IEC_FLAG                  (DSP_UOPID5 + (0x01 << 8))
#define UOP_DSP_SPEED                     (DSP_UOPID5 + (0x02 << 8))
#define UOP_DSP_PROCESSING_MODE           (DSP_UOPID5 + (0x03 << 8))
#define UOP_DSP_IEC_DOWN_SAMPLE           (DSP_UOPID5 + (0x04 << 8))
//#define UOP_DSP_AOUT_REINIT               (DSP_UOPID5 + (0x06 << 8))

// Light added for AVC
#define UOP_DSP_AVC_CONTROL               (DSP_UOPID5 + (0x07 << 8))
#define UOP_DSP_SDET_CONTROL              (DSP_UOPID5 + (0x08 << 8))

// second decoder
#define UOP_DSP_IEC_FLAG2                 (DSP_UOPID5 + (0x11 << 8))
#define UOP_DSP_SPEED_DEC2                (DSP_UOPID5 + (0x12 << 8))
#define UOP_DSP_PROCESSING_MODE_DEC2      (DSP_UOPID5 + (0x13 << 8))
#define UOP_DSP_IEC_DOWN_SAMPLE_DEC2      (DSP_UOPID5 + (0x14 << 8))

// Light added for AVC
#define UOP_DSP_AVC_CONTROL_DEC2         (DSP_UOPID5 + (0x17 << 8))
#define UOP_DSP_SDET_CONTROL_DEC2        (DSP_UOPID5 + (0x18 << 8))

/************************************************************************/
// Pink Noise UOP
/************************************************************************/
#define DSP_UOPID6                        (UOP_DSP + 0x06)
#define UOP_DSP_PINK_NOISE_ON             (DSP_UOPID6 + (0x01 << 8))
#define UOP_DSP_PINK_NOISE_OFF            (DSP_UOPID6 + (0x02 << 8))
#define UOP_DSP_PINK_NOISE_CHANGE         (DSP_UOPID6 + (0x03 << 8))
#define UOP_DSP_PINK_NOISE_PLAY           (DSP_UOPID6 + (0x04 << 8))


/************************************************************************/
// TEST TONE UOP
/************************************************************************/
#define UOP_DSP_TEST_TONE_ON                  (DSP_UOPID6 + (0x01 << 8))
#define UOP_DSP_TEST_TONE_OFF                 (DSP_UOPID6 + (0x02 << 8))
  
#define UOP_DSP_TEST_TONE_SET_FRONT_OPTION    (DSP_UOPID6 + (0x05 << 8))
#define UOP_DSP_TEST_TONE_FRONT_CONFIG        (DSP_UOPID6 + (0x06 << 8))
#define UOP_DSP_TEST_TONE_SET_REAR_OPTION     (DSP_UOPID6 + (0x07 << 8))
#define UOP_DSP_TEST_TONE_REAR_CONFIG         (DSP_UOPID6 + (0x08 << 8))


/************************************************************************/
// LR selection UOP
/************************************************************************/
#define DSP_UOPID7                        (UOP_DSP + 0x07)
#define UOP_DSP_KARAOKE_FLAG              (DSP_UOPID7 + (0x01 << 8))
#define UOP_DSP_LR_MIX_RATIO              (DSP_UOPID7 + (0x02 << 8))
#define UOP_DSP_KARAOKE_FLAG_DEC2         (DSP_UOPID7 + (0x11 << 8))
#define UOP_DSP_LR_MIX_RATIO_DEC2         (DSP_UOPID7 + (0x12 << 8))

/************************************************************************/
// Equalizer UOP
/************************************************************************/
#define DSP_UOPID8                        (UOP_DSP + 0x08)
#define UOP_DSP_EQUALIZER_FLAG            (DSP_UOPID8 + (0x01 << 8)) /*8530 use*/
#define UOP_DSP_EQUALIZER_CHANNEL_GAIN    (DSP_UOPID8 + (0x02 << 8))
#define UOP_DSP_EQUALIZER_CHANNEL_CHANGE  (DSP_UOPID8 + (0x03 << 8))
#define UOP_DSP_SUPER_BASS_DELAY          (DSP_UOPID8 + (0x04 << 8))
#define UOP_DSP_SUPER_BASS_BOOST_GAIN     (DSP_UOPID8 + (0x05 << 8))
#define UOP_DSP_SUPER_BASS_CLEAR_GAIN     (DSP_UOPID8 + (0x06 << 8))
#define UOP_DSP_EQUALIZER_BANDNUM         (DSP_UOPID8 + (0x07 << 8)) /*8530 use*/
#define UOP_DSP_EQUALIZER_GAIN            (DSP_UOPID8 + (0x08 << 8)) /*8530 use*/

// Secondary decoder
#define UOP_DSP_EQUALIZER_FLAG_DEC2            (DSP_UOPID8 + (0x11 << 8))
#define UOP_DSP_EQUALIZER_CHANNEL_GAIN_DEC2    (DSP_UOPID8 + (0x12 << 8))
#define UOP_DSP_EQUALIZER_CHANNEL_CHANGE_DEC2  (DSP_UOPID8 + (0x13 << 8))
#define UOP_DSP_SUPER_BASS_DELAY_DEC2          (DSP_UOPID8 + (0x14 << 8))
#define UOP_DSP_SUPER_BASS_BOOST_GAIN_DEC2     (DSP_UOPID8 + (0x15 << 8))
#define UOP_DSP_SUPER_BASS_CLEAR_GAIN_DEC2     (DSP_UOPID8 + (0x16 << 8))

/************************************************************************/
// 3D Processing UOP
/************************************************************************/
#define DSP_UOPID9                        (UOP_DSP + 0x09)
#define UOP_DSP_PRO_LOGICII_CONFIG        (DSP_UOPID9 + (0x01 << 8))
#define UOP_DSP_PRO_LOGICII_MODE          (DSP_UOPID9 + (0x02 << 8))
#define UOP_DSP_VIRTUAL_SURROUND_FLAG     (DSP_UOPID9 + (0x03 << 8))
#define UOP_DSP_VIRTUAL_SURROUND_GAIN     (DSP_UOPID9 + (0x04 << 8))
#define UOP_DSP_VIRTUAL_SURROUND_WIDE     (DSP_UOPID9 + (0x05 << 8))
#define UOP_DSP_VIRTUAL_SURROUND_DELAY    (DSP_UOPID9 + (0x06 << 8))
#define UOP_DSP_REVERB_FLAG               (DSP_UOPID9 + (0x07 << 8))
#define UOP_DSP_REVERB_GAIN               (DSP_UOPID9 + (0x08 << 8))
#define UOP_DSP_REVERB_BANK               (DSP_UOPID9 + (0x09 << 8))
#define UOP_DSP_NEO6_FLAG                 (DSP_UOPID9 + (0x0C << 8))

// Sencodary decoder
#define UOP_DSP_VIRTUAL_SURROUND_FLAG_DEC2     (DSP_UOPID9 + (0x13 << 8))
#define UOP_DSP_VIRTUAL_SURROUND_GAIN_DEC2     (DSP_UOPID9 + (0x14 << 8))
#define UOP_DSP_VIRTUAL_SURROUND_WIDE_DEC2     (DSP_UOPID9 + (0x15 << 8))
#define UOP_DSP_VIRTUAL_SURROUND_DELAY_DEC2    (DSP_UOPID9 + (0x16 << 8))
#define UOP_DSP_REVERB_FLAG_DEC2               (DSP_UOPID9 + (0x17 << 8))
#define UOP_DSP_REVERB_GAIN_DEC2               (DSP_UOPID9 + (0x18 << 8))

/************************************************************************/
// Decoding option UOP
/************************************************************************/
#define DSP_UOPIDA                        (UOP_DSP + 0x0A)
//DOLBY AC-3 configuration for first decoder
#define UOP_DSP_AC3_KARAOKE_MODE          (DSP_UOPIDA + (0x01 << 8))
#define UOP_DSP_AC3_DUAL_MONO_MODE        (DSP_UOPIDA + (0x02 << 8))
#define UOP_DSP_AC3_COMPRESSION_MODE      (DSP_UOPIDA + (0x03 << 8))
#define UOP_DSP_AC3_DYNAMIC_LOW           (DSP_UOPIDA + (0x04 << 8))
#define UOP_DSP_AC3_DYNAMIC_HIGH          (DSP_UOPIDA + (0x05 << 8))
#define UOP_DSP_AC3_AC3AUTODNMIX          (DSP_UOPIDA + (0x06 << 8))
//DOLBY AC-3 OUTPUT MODE change with Speaker config
//AC3_OUTMODE, Orho
#define UOP_DSP_AC3_OUTMODE               (DSP_UOPIDA + (0x17 << 8))
#define UOP_DSP_AC3_LFEMODE               (DSP_UOPIDA + (0x18 << 8))

// DTS configuration for first decoder
#define UOP_DSP_DTS_FLAG                  (DSP_UOPIDA + (0x07 << 8))
#define UOP_DSP_DTS_DYN_VALUE             (DSP_UOPIDA + (0x08 << 8))

//HDCD
#define UOP_DSP_HDCD_CONFIG               (DSP_UOPIDA + (0x0b << 8))
#define UOP_DSP_HDCD_UPSAMPLING           (DSP_UOPIDA + (0x0c << 8))
#define UOP_DSP_HDCD_DITHER_MODE          (DSP_UOPIDA + (0x0d << 8))
#define UOP_DSP_CDDA_DEEMPH               (DSP_UOPIDA + (0x0e << 8))
#define UOP_DSP_HDCD_AOUT_BLENGTH          (DSP_UOPIDA + (0x0f << 8))

//DOLBY AC-3 configuration for secodn decoder
#define UOP_DSP_AC3_KARAOKE_MODE_DEC2     (DSP_UOPIDA + (0x11 << 8))
#define UOP_DSP_AC3_DUAL_MONO_MODE_DEC2   (DSP_UOPIDA + (0x12 << 8))
#define UOP_DSP_AC3_COMPRESSION_MODE_DEC2 (DSP_UOPIDA + (0x13 << 8))
#define UOP_DSP_AC3_DYNAMIC_LOW_DEC2      (DSP_UOPIDA + (0x14 << 8))
#define UOP_DSP_AC3_DYNAMIC_HIGH_DEC2     (DSP_UOPIDA + (0x15 << 8))
#define UOP_DSP_AC3_AC3AUTODNMIX_DEC2     (DSP_UOPIDA + (0x16 << 8))

#define DSP_UOPIDB                        (UOP_DSP + 0x0B)
// Dolby True HD configuration
#define UOP_DSP_TRUEHD_CONFIG             (DSP_UOPIDB + (0x01 << 8))
#define UOP_DSP_TRUEHD_DRC_CUT            (DSP_UOPIDB + (0x02 << 8))
#define UOP_DSP_TRUEHD_DRC_BOOST          (DSP_UOPIDB + (0x03 << 8))
#define UOP_DSP_TRUEHD_DIALNORM           (DSP_UOPIDB + (0x04 << 8))
//Vorbis uop
#define UOP_DSP_VORBIS_TABLE              (DSP_UOPIDB + (0x05 << 8))

/************************************************************************/
// 4th decoder flow control  UOP
/************************************************************************/
#define DSP_UOPIDC                        (UOP_DSP + 0x0C)
// Dolby True HD configuration
#define UOP_DSP_FS_ACK_DEC4               (DSP_UOPIDC + (0x02 << 8))
#define UOP_DSP_FS_ACK_DEC5               (DSP_UOPIDC + (0x03 << 8))


/************************************************************************/
// Encoding flow control  UOP
/************************************************************************/
#define DSP_UOPIDD                        (UOP_DSP + 0x0D)
#define UOP_DSP_REENC_START               (DSP_UOPIDD + (0x01<<8))
#define UOP_DSP_REENC_STOP                (DSP_UOPIDD + (0x02<<8))

/************************************************************************/
// Re-encoding option UOP
/************************************************************************/
#define DSP_UOPIDE                        (UOP_DSP + 0x0E)
// DDCO
#define UOP_DSP_REENC_AC3_FLAG            (DSP_UOPIDE + (0x01<<8))

//DTS
#define UOP_DSP_REENC_DTS_AMODE           (DSP_UOPIDE + (0x02<<8))
/************************************************************************/
// Transcoding option UOP
/************************************************************************/
// MP3 encoder (CD Ripping)
#define DSP_UOPIDF                        (UOP_DSP + 0x0F)

/************************************************************************/
//PCM connection
/************************************************************************/
#define DSP_UOPID10                       (UOP_DSP + 0x10)
//Aout
#define UOP_DSP_AOUT_ON                   (DSP_UOPID10 + (0x01 << 8))
#define UOP_DSP_AOUT_OFF                  (DSP_UOPID10 + (0x02 << 8))
#define UOP_DSP_AOUT2_ON                  (DSP_UOPID10 + (0x03 << 8))
#define UOP_DSP_AOUT2_OFF                 (DSP_UOPID10 + (0x04 << 8))
#define UOP_DSP_FS_ACK                    (DSP_UOPID10 + (0x05 << 8))
#define UOP_DSP_HDMI_REINIT               (DSP_UOPID10 + (0x06 << 8))
#define UOP_DSP_AOUT_REINIT               (DSP_UOPID10 + (0x07 << 8))

//Primary Mixer Connection
#define UOP_DSP_MIXER_CONNECT             (DSP_UOPID10 + (0x20 << 8))
#define UOP_DSP_MIXER_DISCONNECT          (DSP_UOPID10 + (0x21 << 8))
#define UOP_DSP_MIXER_AVSYNC_START        (DSP_UOPID10 + (0x22 << 8))
#define UOP_DSP_MIXER_AVSYNC_BYPASS       (DSP_UOPID10 + (0x23 << 8))
#define UOP_DSP_MIXER_STEP                (DSP_UOPID10 + (0x24 << 8))
#define UOP_DSP_MIXER_PTS_READY           (DSP_UOPID10 + (0x25 << 8))
#define UOP_DSP_MIXER_BEGIN_PTS           (DSP_UOPID10 + (0x26 << 8))
#define UOP_DSP_MIXER_CANCEL_STEP         (DSP_UOPID10 + (0x27 << 8))
#define UOP_DSP_MIXER_CANCEL_BEGIN_PTS    (DSP_UOPID10 + (0x28 << 8))
#define UOP_DSP_MIXER_DIRECTMIX_SELECT    (DSP_UOPID10 + (0x29 << 8))// For sacd mixing path
#define UOP_DSP_MIXER_END_PTS             (DSP_UOPID10 + (0x2a << 8))
#define UOP_DSP_MIXER_END_PTS_OFF         (DSP_UOPID10 + (0x2b << 8))
#define UOP_DSP_MIXER_STEP_TO_END         (DSP_UOPID10 + (0x2C << 8))

//Secondary Mixer Connection
#define UOP_DSP_SECONDARY_MIXER_CONNECT    (DSP_UOPID10 + (0x30 << 8))
#define UOP_DSP_SECONDARY_MIXER_DISCONNECT (DSP_UOPID10 + (0x31 << 8))
#define UOP_DSP_SECONDARY_MIXER_AVSYNC_START        (DSP_UOPID10 + (0x32 << 8))
#define UOP_DSP_SECONDARY_MIXER_AVSYNC_BYPASS       (DSP_UOPID10 + (0x33 << 8))
#define UOP_DSP_SECONDARY_MIXER_STEP                (DSP_UOPID10 + (0x34 << 8))
#define UOP_DSP_SECONDARY_MIXER_PTS_READY           (DSP_UOPID10 + (0x35 << 8))
#define UOP_DSP_SECONDARY_MIXER_BEGIN_PTS           (DSP_UOPID10 + (0x36 << 8))
#define UOP_DSP_SECONDARY_MIXER_CANCEL_STEP         (DSP_UOPID10 + (0x37 << 8))
#define UOP_DSP_SECONDARY_MIXER_CANCEL_BEGIN_PTS    (DSP_UOPID10 + (0x38 << 8))
#define UOP_DSP_SECONDARY_MIXER_END_PTS             (DSP_UOPID10 + (0x39 << 8))
#define UOP_DSP_SECONDARY_MIXER_END_PTS_OFF         (DSP_UOPID10 + (0x3a << 8))
#define UOP_DSP_SECONDARY_MIXER_STEP_TO_END                (DSP_UOPID10 + (0x3b << 8))

//Tertiary Mixer Connection
#define UOP_DSP_TERTIARY_MIXER_CONNECT       (DSP_UOPID10 + (0x40 << 8))
#define UOP_DSP_TERTIARY_MIXER_DISCONNECT    (DSP_UOPID10 + (0x41 << 8))

//Add the fourth connect and disconnect by mtk40292
#define UOP_DSP_FOURTH_MIXER_CONNECT         (DSP_UOPID10 + (0x42 << 8))
#define UOP_DSP_FOURTH_MIXER_DISCONNECT      (DSP_UOPID10 + (0x43 << 8))
#define UOP_DSP_FRONT_AOUT_RESET             (DSP_UOPID10 + (0x44 << 8))
#define UOP_DSP_REAR_AOUT_RESET              (DSP_UOPID10 + (0x45 << 8))

#define UOP_DSP_GPS_MIX_START                (DSP_UOPID10 + (0x46 << 8))
#define UOP_DSP_GPS_MIX_STOP                 (DSP_UOPID10 + (0x47 << 8))
#define UOP_DSP_GPS_MIX_PAUSE                (DSP_UOPID10 + (0x48 << 8))
#define UOP_DSP_GPS_MIX_RESUME               (DSP_UOPID10 + (0x49 << 8))

#define UOP_DSP_FIFTH_MIXER_CONNECT         (DSP_UOPID10 + (0x4A << 8))
#define UOP_DSP_FIFTH_MIXER_DISCONNECT      (DSP_UOPID10 + (0x4B << 8))

// Post process UOP entry (aud_se_v2)
// [31-------16][15---8][7--0]
//  u4UopOpCode  u4Type  0x1B
#define DSP_UOPID1B                       (UOP_DSP + 0x1B)

// Encoder Used UOP
#define DSP_UOPID1C                       (UOP_DSP + 0x1C)              // -- Water (AUD_RIPPING)
#define UOP_DSP_ENC_START_NORMAL          (DSP_UOPID1C + (0x01<<8))
#define UOP_DSP_ENC_START_HS              (DSP_UOPID1C + (0x02<<8))
#define UOP_DSP_ENC_FLUSH                 (DSP_UOPID1C + (0x03<<8))
#define UOP_DSP_ENC_STOP                  (DSP_UOPID1C + (0x04<<8))

//microphone
#define DSP_UOPID1D                       (UOP_DSP + 0x1D)
#define UOP_DSP_MIC_ON                    (DSP_UOPID1D + (0x01 << 8))
#define UOP_DSP_MIC_OFF                   (DSP_UOPID1D + (0x02 << 8))
#define UOP_DSP_MIC_VOLUME1               (DSP_UOPID1D + (0x03 << 8))
#define UOP_DSP_MIC_VOLUME2               (DSP_UOPID1D + (0x04 << 8))
#define UOP_DSP_MIC_THRESHOLD_MUTE        (DSP_UOPID1D + (0x05 << 8))
#define UOP_DSP_MIC_THRESHOLD_UNMUTE      (DSP_UOPID1D + (0x06 << 8))
#define UOP_DSP_MIC_ECHO_VOLUME           (DSP_UOPID1D + (0x07 << 8))
#define UOP_DSP_MIC_ECHO_FBGAIN           (DSP_UOPID1D + (0x08 << 8))
#define UOP_DSP_MIC_ECHO_DELAY            (DSP_UOPID1D + (0x09 << 8))
#define UOP_DSP_PATH_SETTING              (DSP_UOPID1D + (0x0A << 8))
#define UOP_DSP_KEYSHIFT                  (DSP_UOPID1D + (0x0B << 8))


/************************************************************************/
//Interrupt Messages
/************************************************************************/
#define DSP_UOP_INT                       (UOP_DSP + 0xF0)



