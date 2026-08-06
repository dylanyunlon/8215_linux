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

/******************************************************************************
*[File]                DspShm.h
*[Author]
*[Description]
******************************************************************************/

/* PLEASE NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                                                                            */
/* Strict rules - NO replacement of share memory for backward compatible      */
/*                Until new version for architecture is established           */
/*                                                                            */
/* PLEASE NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef _DSP_SHM_H_
#define _DSP_SHM_H_

#define   DSP_GRP_MAX                   64

/************** System related ********************/
#define   SYSTEM_GRP                    (0 * 32)
#define   W_SYSTEM_CTRL                         (SYSTEM_GRP + 0x02)   // system
#define   W_SYSTEM_SETUP                        (SYSTEM_GRP + 0x04)   // system
#define   W_SYSTEM_SETUP_DEC2                   (SYSTEM_GRP + 0x06)   // system

/************** Mixsound related ********************/
#define   MXSND_GRP                     (1 * 32)
#define   W_MIXSOUND_FLAG                       (MXSND_GRP + 0x00)    // mixed sound
#define   W_MIXSOUND_CFG                        (MXSND_GRP + 0x02)    // mixed sound
#define   D_MIXSOUND_PADDR                      (MXSND_GRP + 0x04)    // mixed sound
#define   D_MIXSOUND_PLEN                       (MXSND_GRP + 0x08)    // mixed sound
#define   D_MIXSOUND_GAIN                       (MXSND_GRP + 0x0C)    // mixed sound

#define   D_TEST_TONE_CHANNEL_ANALOG_FRONT      (MXSND_GRP + 0x14)    // for test tone channel setting
#define   D_TEST_TONE_CHANNEL_ANALOG_REAR       (MXSND_GRP + 0x18)    // for test tone channel setting


/************** Volume related part-1 for first decoder ********************/
#define   VOL_GRP1                      (2 * 32)
#define   D_TRIM_C                              (VOL_GRP1 + 0x00) // first decoder
#define   D_TRIM_L                              (VOL_GRP1 + 0x04) // first decoder
#define   D_TRIM_R                              (VOL_GRP1 + 0x08) // first decoder
#define   D_TRIM_LS                             (VOL_GRP1 + 0x0C) // first decoder
#define   D_TRIM_RS                             (VOL_GRP1 + 0x10) // first decoder
#define   D_TRIM_CH7                            (VOL_GRP1 + 0x14) // first decoder
#define   D_TRIM_CH8                            (VOL_GRP1 + 0x18) // first decoder
#define   D_TRIM_SUB                            (VOL_GRP1 + 0x1C) // first decoder


/************** Volume related part-2 for first decoder ********************/
#define   VOL_GRP2                      (3 * 32)
#define   D_TRIM_LFE                            (VOL_GRP2 + 0x00) // first decoder
#define   D_DIALOGUE                            (VOL_GRP2 + 0x04) // first decoder
#define   D_VOL                                 (VOL_GRP2 + 0x08) // first decoder
#define   B_VOLUPORDER                          (VOL_GRP2 + 0x0C) // first decoder
#define   B_VOLDOWNORDER                        (VOL_GRP2 + 0x0D) // first decoder
#define   B_SOFTMUTEORDER                       (VOL_GRP2 + 0x0E) // first decoder
#define   B_VOLSMOOTHORDER                      (VOL_GRP2 + 0x0F) // first decoder
#define   W_ERRORMUTEBANK                       (VOL_GRP2 + 0x10) // first decoder
#define   B_DRC_AVEORDER                        (VOL_GRP2 + 0x12) // first decoder
#define   B_DRC_ATTACKORDER                     (VOL_GRP2 + 0x13) // first decoder
#define   W_DRC_THRESHOLD                       (VOL_GRP2 + 0x14) // first decoder
#define   W_DRC_MARGIN                          (VOL_GRP2 + 0x16) // first decoder
#define   D_DRC_RE_GAIN                         (VOL_GRP2 + 0x18) // first decoder
#define   B_DRC_FLAG                            (VOL_GRP2 + 0x1C) // first decoder
#define   B_DEC_SOFTMUTEORDER                   (VOL_GRP2 + 0x1D) // first decoder

/************** Ctrl for first decoder ********************/
#define   CTRL_GRP1                     (4 * 32)
#define   B_BANK384NUM                          (CTRL_GRP1 + 0x00) // first decoder
#define   B_BANK576NUM                          (CTRL_GRP1 + 0x01) // first decoder
#define   B_BANK320NUM                          (CTRL_GRP1 + 0x02) // first decoder
#define   B_BANK256NUM                          (CTRL_GRP1 + 0x03) // first decoder
#define   B_DACBIT                              (CTRL_GRP1 + 0x08) // first decoder
#define   B_BIT                                 (CTRL_GRP1 + 0x09)    // first decoder
#define   W_PROCMOD                             (CTRL_GRP1 + 0x0A)    // first decoder
#define   W_INTCTRL                             (CTRL_GRP1 + 0x0E)    // first decoder
#define   D_DECODING_STR_PNT                    (CTRL_GRP1 + 0x10)    // first decoder
#define   D_MUTE_THRESHOLD                      (CTRL_GRP1 + 0x14)    // first decoder
#define   D_AMUTE_THRESHOLD                     (CTRL_GRP1 + 0x18)    // first decoder
#define   D_LEVEL_THRESHOLD                     (CTRL_GRP1 + 0x1C)    // first decoder

/************** Config for first decoder ********************/
#define   CFG_GRP1                      (5 * 32)
#define   W_SPEED                               (CFG_GRP1 + 0x00)    // first decoder
#define   W_KEYLEVEL                            (CFG_GRP1 + 0x02)    // first decoder
#define   D_PLAY_SPEED                          (CFG_GRP1 + 0x04)    // first decoder (0.5~2X)
#define   B_EQFLAG                              (CFG_GRP1 + 0x09) // first decoder
#define   B_EQBANDNUM                           (CFG_GRP1 + 0x0A) // first decoder
#define   B_SBASSDELAYNUM                       (CFG_GRP1 + 0x0B) // first decoder
#define   D_SBASSBOOSTGAIN                      (CFG_GRP1 + 0x0C) // first decoder
#define   D_SBASSCLEARGAIN                      (CFG_GRP1 + 0x10) // first decoder
#define   B_SPECTRUMUP                          (CFG_GRP1 + 0x14) // first decoder
#define   B_SPECTRUMDOWN                        (CFG_GRP1 + 0x15) // first decoder
#define   B_PLAYBACK_DISC_TYPE                  (CFG_GRP1 + 0x16) // For CGMS info: //  0:VCD ; 1:DVD; 2: BD; 3: DIVX
#define   B_PLAYBACK_DVD_CSS_INFO               (CFG_GRP1 + 0x17) // For DVD CSS info
#define   B_BD_AACS_VALID                       (CFG_GRP1 + 0x18)//FOR BD AACS info
#define   B_MEDIA_TYPE_DVDVID_INFO              (CFG_GRP1 + 0x19)//FOR media type dvd video info
#define   B_BD_DOT_VALID                        (CFG_GRP1 + 0x1a) //FOR DOT Valid info
#define   B_BD_ICT_VALID                        (CFG_GRP1 + 0x1b) //FOR ICT Valid info
#define   B_MEDIA_TYPE_FLAG                     (CFG_GRP1 + 0x1c) //bit0:Media type DVD or not. bit1:LPCM or not bit2:HDMV or not
                                                                //bit3:Media type CDDA or not. bit4:Media type SACD or not
                                                                //bit5:Media type BDAV or not
#define   B_CGMS_KEEP_FLAG                      (CFG_GRP1 + 0x1d) // 1 - Keep Cgsm,not to clear ; 0 - Not keep,clear cgms

/************** Bass related for first decoder ********************/
#define   BASS_GRP1                     (6 * 32)
#define   D_SPKCFG                              (BASS_GRP1 + 0x00)    // first decoder
#define   D_SPKCFG_2                            (BASS_GRP1 + 0x04)    // first decoder
#define   W_CUTOFF_FREQ                         (BASS_GRP1 + 0x08)    // first decoder
#define   W_CHDELAY_C                           (BASS_GRP1 + 0x0a)    // first decoder
#define   W_CHDELAY_L                           (BASS_GRP1 + 0x0C)    // first decoder
#define   W_CHDELAY_R                           (BASS_GRP1 + 0x0E)    // first decoder
#define   W_CHDELAY_LS                          (BASS_GRP1 + 0x10)    // first decoder
#define   W_CHDELAY_RS                          (BASS_GRP1 + 0x12)    // first decoder
#define   W_CHDELAY_CH7                         (BASS_GRP1 + 0x14)    // first decoder
#define   W_CHDELAY_CH8                         (BASS_GRP1 + 0x16)    // first decoder
#define   W_CHDELAY_SUB                         (BASS_GRP1 + 0x18)    // first decoder
#define   W_CHDELAY_CH9                         (BASS_GRP1 + 0x1A)    // first decoder
#define   W_CHDELAY_CH10                        (BASS_GRP1 + 0x1c)    // first decoder
#define   B_AUD_CHANN_MODE                      (BASS_GRP1 + 0x1e)    // first decoder
#define   B_HTS_HDMI_CHANNEL_MODE               (BASS_GRP1 + 0x1f)    // first decoder
/************** Decoder option for first decoder ********************/
#define   HDMI_GRP1                     (7 * 32)
#define   D_HDMI_SPKCFG                         (HDMI_GRP1 + 0x00)
#define   D_HDMI_SPKCFG2                        (HDMI_GRP1 + 0x04)
#define   W_HDMI_CHDELAY_C                      (HDMI_GRP1 + 0x08)    // first decoder
#define   W_HDMI_CHDELAY_L                      (HDMI_GRP1 + 0x0a)    // first decoder
#define   W_HDMI_CHDELAY_R                      (HDMI_GRP1 + 0x0C)    // first decoder
#define   W_HDMI_CHDELAY_LS                     (HDMI_GRP1 + 0x0E)    // first decoder
#define   W_HDMI_CHDELAY_RS                     (HDMI_GRP1 + 0x10)    // first decoder
#define   W_HDMI_CHDELAY_CH7                    (HDMI_GRP1 + 0x12)    // first decoder
#define   W_HDMI_CHDELAY_CH8                    (HDMI_GRP1 + 0x14)    // first decoder
#define   W_HDMI_CHDELAY_SUB                    (HDMI_GRP1 + 0x16)    // first decoder
#define   W_FRONT_FREQ                          (HDMI_GRP1 + 0x18)    // front set: front spk size HPF
#define   W_CENTER_FREQ                         (HDMI_GRP1 + 0x1A)    // front set: center spk size HPF
#define   W_REAR_FREQ                           (HDMI_GRP1 + 0x1C)    // front set: rear spk size HPF
#define   W_SUB_FREQ                            (HDMI_GRP1 + 0x1E)    // front set: sub spk size LPF

/************** IEC related for first decoder ********************/
#define   IEC_GRP1                      (8 * 32)
#define   B_IECFLAG                             (IEC_GRP1 + 0x00) // first decoder
#define   B_IEC_MUTE                            (IEC_GRP1 + 0x01) // first decoder
#define   B_IEC_PCMCH                           (IEC_GRP1 + 0x02) // first decoder
#define   B_IEC_OPTION                          (IEC_GRP1 + 0x03) // first decoder
#define   B_IEC_MAX_FREQ                        (IEC_GRP1 + 0x04) // first decoder
#define   B_IECLATENCY                          (IEC_GRP1 + 0x05) // first decoder
#define   B_IEC_RAWMUTE                         (IEC_GRP1 + 0x06) // first decoder
#define   B_IEC_CSS_LIMIT                       (IEC_GRP1 + 0x07) // first decoder
#define   B_IEC_AC3                             (IEC_GRP1 + 0x08) // first decoder
#define   B_IEC_MP2                             (IEC_GRP1 + 0x09) // first decoder
#define   B_IEC_CDDA                            (IEC_GRP1 + 0x0A) // first decoder
#define   B_IEC_DTS                             (IEC_GRP1 + 0x0B) // first decoder
#define   B_IEC_TRUEHD                          (IEC_GRP1 + 0x0C) // first decoder
#define   B_IEC_WMAPRO                          (IEC_GRP1 + 0x0D) // first decoder
#define   B_IEC_AAC                             (IEC_GRP1 + 0x0E) // first decoder
#define   B_IEC_BIT_RES                         (IEC_GRP1 + 0x0F) // IEC1 output bit resolution (16, 20, 24)
#define   W_AC3_SPKCFG                          (IEC_GRP1 + 0x10) // first decoder
#define   W_MP2_SPKCFG                          (IEC_GRP1 + 0x12) // first decoder
#define   W_SACD_SPKCFG                         (IEC_GRP1 + 0x14) // first decoder
#define   W_DTS_SPKCFG                          (IEC_GRP1 + 0x16) // first decoder

//#define   B_IECFLAG2                   ((IEC_GRP1 * 32) + 0x19) // first decoder
/************** 3D(Reverb) for first decoder ********************/
#define   DSP3D_GRP1                    (9 * 32)
#define   D_REVERBMEMADDR                       (DSP3D_GRP1 + 0x00)   // first decoder
#define   B_REVERBFLAG                          (DSP3D_GRP1 + 0x04)   // first decoder
#define   D_REVERBGAIN                          (DSP3D_GRP1 + 0x08)   // first decoder
#define   B_REVERBBANK0                         (DSP3D_GRP1 + 0x0C)   // first decoder
#define   B_REVERBBANK1                         (DSP3D_GRP1 + 0x0D)   // first decoder
#define   B_REVERBBANK2                         (DSP3D_GRP1 + 0x0E)   // first decoder
#define   B_REVERBBANK3                         (DSP3D_GRP1 + 0x0F)   // first decoder
#define   D_REVERBIIRA                          (DSP3D_GRP1 + 0x10)   // first decoder
#define   D_REVERBIIRB                          (DSP3D_GRP1 + 0x14)   // first decoder
#define   D_REVERBCOEFADDR                      (DSP3D_GRP1 + 0x18)   // first decoder

/************** 3D(VSS/PL2) for first decoder ********************/
#define   DSP3D_GRP2                    (10 * 32)
#define   B_VSURRFLAG                           (DSP3D_GRP2 + 0x04)   // first decoder
#define   B_VSURRDELAYNUM                       (DSP3D_GRP2 + 0x05)   // first decoder
#define   D_VSURRGAIN                           (DSP3D_GRP2 + 0x08)   // first decoder
#define   D_VSURRWIDEGAIN                       (DSP3D_GRP2 + 0x0C)   // first decoder
#define   B_VSSMODE                             (DSP3D_GRP2 + 0x10)   // first decoder
#define   B_TVSMODE                             (DSP3D_GRP2 + 0x11)   // first decoder
#define   B_DH2MODE                             (DSP3D_GRP2 + 0x12)   // first decoder
#define   B_SPATIALDEGREE                       (DSP3D_GRP2 + 0x13)   // first decoder

/************** 3D(PL2/Neo6) for second decoder ********************/
#define   DSP3D_GRP3                    (11 * 32)
#define   W_PLSURRDELAY                         (DSP3D_GRP3 + 0x04)   // first decoder
#define   W_PLIICONFIG                          (DSP3D_GRP3 + 0x06)   // first decoder
#define   W_PLIIMODE                            (DSP3D_GRP3 + 0x08)   // first decoder
#define   B_NEO6_FLAG                           (DSP3D_GRP3 + 0x0A)   // first decoder
//bit0: Neo6 on(1)/off(0)
//bit1: Music mode(0)/Cinema mode(1)
//bit2(disabled now): Not wide mode(0)/ Wide mode(1)
#define   D_NEO6_TABLE                          (DSP3D_GRP3 + 0x0C)   // first decoder
#define   D_NEO6_ADDR                           (DSP3D_GRP3 + 0x10)   // first decoder
#define   W_NEO6_CGAIN                          (DSP3D_GRP3 + 0x14)   // first decoder
#define   B_NEO6_DECTRIG                        (DSP3D_GRP3 + 0x16)   // first decoder
#define   W_CSII_CONFIG                         (DSP3D_GRP3 + 0x18)   // first decoder
#define   D_CSII_MODE                           (DSP3D_GRP3 + 0x1C)   // first decoder
//Decoder trigger for UI setting
//bit0: Neo6 on(1)/off(0)         

/************** 3D(QSound) for second decoder ********************/
#define   DSP3D_GRP4                    (12 * 32)
#define   W_QSOUNDMODE                          (DSP3D_GRP4 + 0x0)   //QSURR
#define   D_QSX_FCDROP                          (DSP3D_GRP4 + 0x4)   //QXpander front center drop
#define   D_QSX_SCDROP                          (DSP3D_GRP4 + 0x8)   //QXpander surround center drop
#define   D_QSX_FSPREAD                         (DSP3D_GRP4 + 0xC)   //QXpander front spread
#define   D_QSX_SSPREAD                         (DSP3D_GRP4 + 0x10)  //QXpadner surround spread

/************** Equalizer for first decoder ********************/
#define   DSP_EQ_GRP1                   (13 * 32)
#define   D_C_DRY                               (DSP_EQ_GRP1 + 0x00)  // first decoder
#define   D_C_BAND1                             (DSP_EQ_GRP1 + 0x04)  // first decoder
#define   D_C_BAND2                             (DSP_EQ_GRP1 + 0x08)  // first decoder
#define   D_C_BAND3                             (DSP_EQ_GRP1 + 0x0C)  // first decoder
#define   D_C_BAND4                             (DSP_EQ_GRP1 + 0x10)  // first decoder
#define   D_C_BAND5                             (DSP_EQ_GRP1 + 0x14)  // first decoder
#define   D_C_BAND6                             (DSP_EQ_GRP1 + 0x18)  // first decoder
#define   D_C_BAND7                             (DSP_EQ_GRP1 + 0x1C)  // first decoder

#define   DSP_EQ_GRP2                   (14 * 32)
#define   D_C_BAND8                             (DSP_EQ_GRP2 + 0x00)  // first decoder
#define   D_C_BAND9                             (DSP_EQ_GRP2 + 0x04)  // first decoder
#define   D_C_BAND10                            (DSP_EQ_GRP2 + 0x08)  // first decoder
#define   D_L_DRY                               (DSP_EQ_GRP2 + 0x0C)  // first decoder
#define   D_L_BAND1                             (DSP_EQ_GRP2 + 0x10)  // first decoder
#define   D_L_BAND2                             (DSP_EQ_GRP2 + 0x14)  // first decoder
#define   D_L_BAND3                             (DSP_EQ_GRP2 + 0x18)  // first decoder
#define   D_L_BAND4                             (DSP_EQ_GRP2 + 0x1C)  // first decoder

#define   DSP_EQ_GRP3                   (15 * 32)
#define   D_L_BAND5                             (DSP_EQ_GRP3 + 0x00)  // first decoder
#define   D_L_BAND6                             (DSP_EQ_GRP3 + 0x04)  // first decoder
#define   D_L_BAND7                             (DSP_EQ_GRP3 + 0x08)  // first decoder
#define   D_L_BAND8                             (DSP_EQ_GRP3 + 0x0C)  // first decoder
#define   D_L_BAND9                             (DSP_EQ_GRP3 + 0x10)  // first decoder
#define   D_L_BAND10                            (DSP_EQ_GRP3 + 0x14)  // first decoder
#define   D_R_DRY                               (DSP_EQ_GRP3 + 0x18)  // first decoder
#define   D_R_BAND1                             (DSP_EQ_GRP3 + 0x1C)  // first decoder

#define   DSP_EQ_GRP4                   (16 * 32)
#define   D_R_BAND2                             (DSP_EQ_GRP4 + 0x00)  // first decoder
#define   D_R_BAND3                             (DSP_EQ_GRP4 + 0x04)  // first decoder
#define   D_R_BAND4                             (DSP_EQ_GRP4 + 0x08)  // first decoder
#define   D_R_BAND5                             (DSP_EQ_GRP4 + 0x0C)  // first decoder
#define   D_R_BAND6                             (DSP_EQ_GRP4 + 0x10)  // first decoder
#define   D_R_BAND7                             (DSP_EQ_GRP4 + 0x14)  // first decoder
#define   D_R_BAND8                             (DSP_EQ_GRP4 + 0x18)  // first decoder
#define   D_R_BAND9                             (DSP_EQ_GRP4 + 0x1C)  // first decoder

#define   DSP_EQ_GRP5                   (17 * 32)
#define   D_R_BAND10                            (DSP_EQ_GRP5 + 0x00)  // first decoder
#define   D_LS_DRY                              (DSP_EQ_GRP5 + 0x04)  // first decoder
#define   D_LS_BAND1                            (DSP_EQ_GRP5 + 0x08)  // first decoder
#define   D_LS_BAND2                            (DSP_EQ_GRP5 + 0x0C)  // first decoder
#define   D_LS_BAND3                            (DSP_EQ_GRP5 + 0x10)  // first decoder
#define   D_LS_BAND4                            (DSP_EQ_GRP5 + 0x14)  // first decoder
#define   D_LS_BAND5                            (DSP_EQ_GRP4 + 0x18)  // first decoder
#define   D_LS_BAND6                            (DSP_EQ_GRP4 + 0x1C)  // first decoder

#define   DSP_EQ_GRP6                   (18 * 32)
#define   D_LS_BAND7                            (DSP_EQ_GRP6 + 0x00)  // first decoder
#define   D_LS_BAND8                            (DSP_EQ_GRP6 + 0x04)  // first decoder
#define   D_LS_BAND9                            (DSP_EQ_GRP6 + 0x08)  // first decoder
#define   D_LS_BAND10                           (DSP_EQ_GRP6 + 0x0C)  // first decoder
#define   D_RS_DRY                              (DSP_EQ_GRP6 + 0x10)  // first decoder
#define   D_RS_BAND1                            (DSP_EQ_GRP6 + 0x14)  // first decoder
#define   D_RS_BAND2                            (DSP_EQ_GRP6 + 0x18)  // first decoder
#define   D_RS_BAND3                            (DSP_EQ_GRP6 + 0x1C)  // first decoder

#define   DSP_EQ_GRP7                   (19 * 32)
#define   D_RS_BAND4                            (DSP_EQ_GRP7 + 0x00)  // first decoder
#define   D_RS_BAND5                            (DSP_EQ_GRP7 + 0x04)  // first decoder
#define   D_RS_BAND6                            (DSP_EQ_GRP7 + 0x08)  // first decoder
#define   D_RS_BAND7                            (DSP_EQ_GRP7 + 0x0C)  // first decoder
#define   D_RS_BAND8                            (DSP_EQ_GRP7 + 0x10)  // first decoder
#define   D_RS_BAND9                            (DSP_EQ_GRP7 + 0x14)  // first decoder
#define   D_RS_BAND10                           (DSP_EQ_GRP7 + 0x18)  // first decoder
#define   D_CH7_DRY                             (DSP_EQ_GRP7 + 0x1C)  // first decoder

#define   DSP_EQ_GRP8                   (20 * 32)
#define   D_CH7_BAND1                           (DSP_EQ_GRP8 + 0x00)  // first decoder
#define   D_CH7_BAND2                           (DSP_EQ_GRP8 + 0x04)  // first decoder
#define   D_CH7_BAND3                           (DSP_EQ_GRP8 + 0x08)  // first decoder
#define   D_CH7_BAND4                           (DSP_EQ_GRP8 + 0x0C)  // first decoder
#define   D_CH7_BAND5                           (DSP_EQ_GRP8 + 0x10)  // first decoder
#define   D_CH7_BAND6                           (DSP_EQ_GRP8 + 0x14)  // first decoder
#define   D_CH7_BAND7                           (DSP_EQ_GRP8 + 0x18)  // first decoder
#define   D_CH7_BAND8                           (DSP_EQ_GRP8 + 0x1C)  // first decoder

#define   DSP_EQ_GRP9                   (21 * 32)
#define   D_CH7_BAND9                           (DSP_EQ_GRP9 + 0x00)  // first decoder
#define   D_CH7_BAND10                          (DSP_EQ_GRP9 + 0x04)  // first decoder
#define   D_CH8_DRY                             (DSP_EQ_GRP9 + 0x08)  // first decoder
#define   D_CH8_BAND1                           (DSP_EQ_GRP9 + 0x0C)  // first decoder
#define   D_CH8_BAND2                           (DSP_EQ_GRP9 + 0x10)  // first decoder
#define   D_CH8_BAND3                           (DSP_EQ_GRP9 + 0x14)  // first decoder
#define   D_CH8_BAND4                           (DSP_EQ_GRP9 + 0x18)  // first decoder
#define   D_CH8_BAND5                           (DSP_EQ_GRP9 + 0x1C)  // first decoder

#define   DSP_EQ_GRP10                  (22 * 32)
#define   D_CH8_BAND6                           (DSP_EQ_GRP10 + 0x00) // first decoder
#define   D_CH8_BAND7                           (DSP_EQ_GRP10 + 0x04) // first decoder
#define   D_CH8_BAND8                           (DSP_EQ_GRP10 + 0x08) // first decoder
#define   D_CH8_BAND9                           (DSP_EQ_GRP10 + 0x0C) // first decoder
#define   D_CH8_BAND10                          (DSP_EQ_GRP10 + 0x10) // first decoder

/************** AVC for first decoder ********************/
// Light added for AVC         
#define   DSP_AVC_GRP1                  (23 * 32)
#define   W_AVC_TARGET_LEV                      (DSP_AVC_GRP1 + 0x00)
#define   W_AVC_SILENCE_LEV                     (DSP_AVC_GRP1 + 0x02)
#define   W_AVC_MAX_GAIN_UP                     (DSP_AVC_GRP1 + 0x04)
#define   W_AVC_MAX_GAIN_DOWN                   (DSP_AVC_GRP1 + 0x06)
#define   W_AVC_FLAG                            (DSP_AVC_GRP1 + 0x08)
#define   W_AVC_ATTACK_THRES                    (DSP_AVC_GRP1 + 0x0a)
#define   W_AVC_ADJUST_RATE                     (DSP_AVC_GRP1 + 0x0c)
#define   D_SDET_SILENCE_LEVEL                  (DSP_AVC_GRP1 + 0x10)
#define   W_SDET_SILENCE_PERIOD                 (DSP_AVC_GRP1 + 0x14)

/************** PTS/STC for first decoder *******************/
#define   DSP_STC_GRP1                  (24 * 32)
#define   D_1ST_PTS_PRS_PNT                     (DSP_STC_GRP1 + 0x00) // first decoder
#define   W_1ST_PTS_STCH                        (DSP_STC_GRP1 + 0x04) // first decoder
#define   W_1ST_PTS_STCL                        (DSP_STC_GRP1 + 0x06) // first decoder
#define   D_STC_DIFF_LO                         (DSP_STC_GRP1 + 0x08) // first decoder
#define   D_STC_DIFF_HI                         (DSP_STC_GRP1 + 0x0c) // first decoder
#define   D_STC_DIFF_WS                         (DSP_STC_GRP1 + 0x10) // first decoder
#define   D_STC_THRESHOLD                       (DSP_STC_GRP1 + 0x14) // first decoder

/************** decoder control first decoder ********************/
#define   DECODER_GRP1                  (25 * 32)
#define   W_AC3KARAMOD                          (DECODER_GRP1 + 0)      //AC3
#define   W_AC3DUALMODE                         (DECODER_GRP1 + 0x02)
#define   W_AC3COMPMOD                          (DECODER_GRP1 + 0x04)
#define   D_AC3DYN_LOW                          (DECODER_GRP1 + 0x08)
#define   D_AC3DYN_HIGH                         (DECODER_GRP1 + 0x0C)
#define   B_AC3AUTODNMIX                        (DECODER_GRP1 + 0x10)
#define   B_EAC3BONDING                         (DECODER_GRP1 + 0x11)
//AC3_OUTMODE, Orho
#define   B_AC3OUTMODE                          (DECODER_GRP1 + 0x12)
#define   B_AC3LFEMODE                          (DECODER_GRP1 + 0x13)
#define   W_DTSFLAG                             (DECODER_GRP1 + 0x14)   //DTS
/*
Bit 0 : DRC On/Off
Bit 1 : DTSCD format (14 bit mode)
Bit 2 : DTSES ON/ OFF
Bit 3 : DISCRETE/ MATRIX
Bit 4 : 96KHz ON/ OFF
Bit 5 : DTS-HD High Resolution support
Bit 6 : DTS-HD Master Audio support
Bit 7 : DTS Audio Presentation Select
Bit 8 : DTS Audio Presentation Select2
Bit 9 : Allow Remapping(0)/No Remapping(1)
*/
#define   D_DTS_DYN_VALUE                        (DECODER_GRP1 + 0x18)
#define   B_AAC_CHK_FRM_NUM                     (DECODER_GRP1 + 0x1C)  //AAC 
#define   B_AACDECTYPE                          (DECODER_GRP1 + 0x1D)  //AAC Dec type 0-aacPlus 1-aac;
#define   B_AACMUTEFLAG                         (DECODER_GRP1 + 0x1E)  //AAC
#define   B_AACDUALMODE                         (DECODER_GRP1 + 0x1F)

/************** decoder control first decoder ********************/
#define   DECODER_GRP2                  (26 * 32)
#define   B_MPEGDRC                             (DECODER_GRP2 + 0)      //MPEG
#define   W_MPEGERRDET                          (DECODER_GRP2 + 0x02)
#define   W_HDCDCONFIG                          (DECODER_GRP2 + 0x08)   //HDCD
#define   W_HDCDUPSAMPLING                      (DECODER_GRP2 + 0x0A)
#define   W_HDCDDITHERMODE                      (DECODER_GRP2 + 0x0C)
#define   W_HDCDAOUTBLENGTH                        (DECODER_GRP2 + 0x0E)
#define   W_WMAPACKET                           (DECODER_GRP2 + 0x10)   //WMA
#define   W_VORBISPACKET                        (DECODER_GRP2 + 0x12)   //VORBIS
#define   B_SACD_IP_CHNUM                       (DECODER_GRP2 + 0x14)   //SACD
#define   B_SACD_IP_DSD_MODE                    (DECODER_GRP2 + 0x15)
//0: DST-coded; 1: PlainDSD
#define   B_SACD_TIMECODE_EN                    (DECODER_GRP2 + 0x16)
#define   B_SACD_OUTMODE_I2S                    (DECODER_GRP2 + 0x17)
//0: DSD output; 1: PCM output; 2: DST output
#define   B_SACD_OUTMODE_HDMI                   (DECODER_GRP2 + 0x18)
//0: DSD output; 1: PCM output; 2: DST output
#define   B_SACD_DOWNSAMPLE_I2S                 (DECODER_GRP2 + 0x19)
//-1: 44k; 0: 88k; 1: 176k
#define   B_SACD_DOWNSAMPLE_HDMI                (DECODER_GRP2 + 0x1A)
//-1: 44k; 0: 88k; 1: 176k
#define   W_PINKNOISE                           (DECODER_GRP2 + 0x1B)   //Pink noise
#define   B_SACD_TABLE_SELECT                   (DECODER_GRP2 + 0x1D)
//0x0: [-3dB for I2S, -3dB for HDMI], 0x11: [0dB for I2S, 0dB for HDMI]
#define   B_SACD_OUT_CHNUM_HDMI                 (DECODER_GRP2 + 0x1E)

/************** decoder control for new codec and secondary decoder ********************/
#define   DECODER_GRP3                  (27 * 32)
#define   W_TRUEHD_CONFIG                        (DECODER_GRP3 + 0x0)      //True HD
#define   D_TRUEHD_CUT                            (DECODER_GRP3 + 0x04)
#define   D_TRUEHD_BOOST                        (DECODER_GRP3 + 0x08)
#define   D_TRUEHD_DIALNORM_REF                    (DECODER_GRP3 + 0x0C)
#define   B_BDLPCM_CHANNEL_ASSIGNMENT            (DECODER_GRP3 + 0x10)
#define   B_BDLPCM_FREQUENCY                    (DECODER_GRP3 + 0x11)
#define   B_BDLPCM_Q                            (DECODER_GRP3 + 0x12)
#define   B_BDLPCM_ON                            (DECODER_GRP3 + 0x13)
#define   B_LPCM_DEC_TYPE                       (DECODER_GRP3 + 0x14)
#define   B_ADPCM_CH_NUM                        (DECODER_GRP3 + 0x15)
#define   W_ADPCM_BLOCK_ALIGN                   (DECODER_GRP3 + 0x16)
#define   B_LPCM_DEEMPHASIS                     (DECODER_GRP3 + 0x18)
#define   B_LPCM_DLNA_FLAG                      (DECODER_GRP3 + 0x19)
#define   B_LPCM_BIT_SHIFT                      (DECODER_GRP3 + 0x1A)
#define   B_LPCM_DRC_VALUE                      (DECODER_GRP3 + 0x1B)
#define   B_LPCM_DRC_FLAG                       (DECODER_GRP3 + 0x1C)
#define   B_LPCM_8BIT_SIGNED                    (DECODER_GRP3 + 0x1D)

/************** Volume related part-1 for second decoder ********************/
#define   VOL_GRP3                      (28 * 32)
#define   D_TRIM_C_DEC2                         (VOL_GRP3 + 0x00) // second decoder -- dummy
#define   D_TRIM_L_DEC2                         (VOL_GRP3 + 0x04) // second decoder
#define   D_TRIM_R_DEC2                         (VOL_GRP3 + 0x08) // second decoder
#define   B_SEC_AUDIO_INFO                      (VOL_GRP3 + 0x0C) // second decoder

/************** Volume related part-2 for second decoder ********************/
#define   VOL_GRP4                      (29 * 32)
#define   D_TRIM_LFE_DEC2                       (VOL_GRP4 + 0x00) // second decoder
#define   D_DIALOGUE_DEC2                       (VOL_GRP4 + 0x04) // second decoder
#define   D_VOL_DEC2                            (VOL_GRP4 + 0x08) // second decoder
#define   B_VOLUPORDER_DEC2                     (VOL_GRP4 + 0x0C) // second decoder
#define   B_VOLDOWNORDER_DEC2                   (VOL_GRP4 + 0x0D) // second decoder
#define   B_SOFTMUTEORDER_DEC2                  (VOL_GRP4 + 0x0E) // second decoder
#define   W_ERRORMUTEBANK_DEC2                  (VOL_GRP4 + 0x10) // second decoder
#define   B_DRC_AVEORDER_DEC2                   (VOL_GRP4 + 0x12) // second decoder
#define   B_DRC_ATTACKORDER_DEC2                (VOL_GRP4 + 0x13) // second decoder
#define   W_DRC_THRESHOLD_DEC2                  (VOL_GRP4 + 0x14) // second decoder
#define   W_DRC_MARGIN_DEC2                     (VOL_GRP4 + 0x16) // second decoder
#define   D_DRC_RE_GAIN_DEC2                    (VOL_GRP4 + 0x18) // second decoder

/************** Ctrl for second decoder ********************/
#define   CTRL_GRP2                     (30 * 32)
#define   W_SPEED_DEC2                          (CTRL_GRP2 + 0x00)    // second decoder
#define   W_PROCMOD_DEC2                        (CTRL_GRP2 + 0x02)    // second decoder
#define   W_PINKNOISE_DEC2                      (CTRL_GRP2 + 0x04)    // second decoder
#define   B_BIT_DEC2                            (CTRL_GRP2 + 0x06)    // second decoder
#define   B_KARAFLAG_DEC2                       (CTRL_GRP2 + 0x07)    // second decoder
#define   D_LRMIXRATIO_DEC2                     (CTRL_GRP2 + 0x08)    // second decoder
#define   D_DECODING_STR_PNT_DEC2               (CTRL_GRP2 + 0x1C)    // second decoder

/************** Config for second decoder ********************/
#define   CFG_GRP2                      (31 * 32)

#define   B_BANK384NUM_DEC2                     (CFG_GRP2 + 0x00) // second decoder
#define   B_BANK576NUM_DEC2                     (CFG_GRP2 + 0x01) // second decoder
#define   B_BANK320NUM_DEC2                     (CFG_GRP2 + 0x02) // second decoder
#define   B_BANK256NUM_DEC2                     (CFG_GRP2 + 0x03) // second decoder
#define   D_MEMBACKUPADDR_DEC2                  (CFG_GRP2 + 0x04) // second decoder
#define   B_DACBIT_DEC2                         (CFG_GRP2 + 0x08) // second decoder
/* currently not support */
#define   B_EQFLAG_DEC2                         (CFG_GRP2 + 0x09)      // second decoder
#define   B_EQBANDNUM_DEC2                      (CFG_GRP2 + 0x0A)      // second decoder
#define   B_SBASSDELAYNUM_DEC2                  (CFG_GRP2 + 0x0B)      // second decoder
#define   D_SBASSBOOSTGAIN_DEC2                 (CFG_GRP2 + 0x0C)      // second decoder
#define   D_SBASSCLEARGAIN_DEC2                 (CFG_GRP2 + 0x10)      // second decoder

/************** Decoder option for second decoder**************/
#define   DEC_GRP2                      (32 * 32)
#define   W_AC3KARAMOD_DEC2                     (DEC_GRP2 + 0x00)
#define   W_AC3DUALMODE_DEC2                    (DEC_GRP2 + 0x02)
#define   W_AC3COMPMOD_DEC2                     (DEC_GRP2 + 0x04)
#define   D_AC3DYN_LOW_DEC2                     (DEC_GRP2 + 0x08)
#define   D_AC3DYN_HIGH_DEC2                    (DEC_GRP2 + 0x0C)
#define   B_AC3AUTODNMIX_DEC2                   (DEC_GRP2 + 0x10)
#define   B_MPEGDRC_DEC2                        (DEC_GRP2 + 0x11)
#define   W_MPEGERRDET_DEC2                        (DEC_GRP2 + 0x12)
#define   W_DTSFLAG_DEC2                        (DEC_GRP2 + 0x14)


/************** Bass related for second decoder ********************/
#define   BASS_GRP2                     (33 * 32)
#define   D_SPKCFG_DEC2                         (BASS_GRP2 + 0x00)    // second decoder
#define   B_CUTOFF_FREQ_DEC2                    (BASS_GRP2 + 0x04)    // second decoder
#define   W_CHDELAY_C_DEC2                      (BASS_GRP2 + 0x08)    // second decoder -- dummy
#define   W_CHDELAY_L_DEC2                      (BASS_GRP2 + 0x0A)    // second decoder
#define   W_CHDELAY_R_DEC2                      (BASS_GRP2 + 0x0C)    // second decoder
#define   W_AC3_SPKCFG_DEC2                     (BASS_GRP2 + 0x1C)    // second decoder
#define   W_MP2_SPKCFG_DEC2                     (BASS_GRP2 + 0x1E)    // second decoder

/************** IEC related for first decoder ********************/
#define   IEC_GRP2                      (34 * 32)
#define   B_IECFLAG2                            (IEC_GRP2 + 0x00) // second decoder
#define   B_IEC2_MUTE                           (IEC_GRP2 + 0x01) // second decoder
#define   B_IEC2_PCMCH                          (IEC_GRP2 + 0x02) // second decoder
#define   B_IEC2_OPTION                         (IEC_GRP2 + 0x03) // second decoder
#define   B_IEC2_MAX_FREQ                       (IEC_GRP2 + 0x04) // second decoder
#define   B_IECLATENCY2                         (IEC_GRP1 + 0x05) // second decoder
#define   B_IEC_MP2_DEC2                        (IEC_GRP2 + 0x06) // second decoder
#define   B_IEC_AC3_DEC2                        (IEC_GRP2 + 0x07) // second decoder

/************** Equalizer for second decoder ********************/
#define   DSP_EQ_GRP11                  (35 * 32)
#define   DSP_EQ_BAND_START_DEC2                (DSP_EQ_GRP11 * 32)  // second decoder
/* currently not support */
#define   D_L_DRY_DEC2                          (DSP_EQ_GRP11 + 0x00)  // second decoder

// M1 Mixing parameters from BDJ
#define   DSP_M1_MIXING_GRP1            (36 * 32)
#define   B_BDJ_PAN_CONTROL                     (DSP_M1_MIXING_GRP1 + 0x00) // 1: Metadata on( from secondary); 0:Metadata off(from BDJ)
#define   B_2ND_AUDIO_ENABLE_MW1                (DSP_M1_MIXING_GRP1 + 0x01) // 1: 2nd audio enable; 0: disable  (from middleware, used by ASH)
#define   B_2ND_AUDIO_ENABLE_MW2                (DSP_M1_MIXING_GRP1 + 0x02) // 1: 2nd audio enable; 0: disable  (from middleware, used by SCC)
#define   B_2ND_AUDIO_ENABLE_UI                 (DSP_M1_MIXING_GRP1 + 0x03) // 1: 2nd audio enabled by UI; 0: disabled by UI
#define   B_BUTTON_SOUND_ENABLE_UI              (DSP_M1_MIXING_GRP1 + 0x07) // 1: UI enabled interactive audio; 0: UI disabled interactive audio
#define   D_BDJ_PRI_GAIN                        (DSP_M1_MIXING_GRP1 + 0x08) // Primary gain from BDJ
#define   D_BDJ_SEC_GAIN                        (DSP_M1_MIXING_GRP1 + 0x0C) // Secondary gain from BDJ

#define   DSP_M1_MIXING_GRP2            (37 * 32)
#define   D_BDJ_PAN_PARA_C                      (DSP_M1_MIXING_GRP2 + 0x00)
#define   D_BDJ_PAN_PARA_L                      (DSP_M1_MIXING_GRP2 + 0x04)
#define   D_BDJ_PAN_PARA_R                      (DSP_M1_MIXING_GRP2 + 0x08)
#define   D_BDJ_PAN_PARA_Ls                     (DSP_M1_MIXING_GRP2 + 0x0C)
#define   D_BDJ_PAN_PARA_Rs                     (DSP_M1_MIXING_GRP2 + 0x10)
#define   D_BDJ_PAN_PARA_L_PRI_2CH              (DSP_M1_MIXING_GRP2 + 0x14)
#define   D_BDJ_PAN_PARA_R_PRI_2CH              (DSP_M1_MIXING_GRP2 + 0x18)

/************** AVC for second decoder ********************/
// Light added for AVC
#define   DSP_AVC_GRP2                  (38 * 32)
#define   W_AVC_TARGET_LEV_DEC2                 (DSP_AVC_GRP2 + 0x00)
#define   W_AVC_SILENCE_LEV_DEC2                (DSP_AVC_GRP2 + 0x02)
#define   W_AVC_MAX_GAIN_UP_DEC2                (DSP_AVC_GRP2 + 0x04)
#define   W_AVC_MAX_GAIN_DOWN_DEC2              (DSP_AVC_GRP2 + 0x06)
#define   W_AVC_FLAG_DEC2                       (DSP_AVC_GRP2 + 0x08)
#define   W_AVC_ATTACK_THRES_DEC2               (DSP_AVC_GRP2 + 0x0a)
#define   W_AVC_ADJUST_RATE_DEC2                (DSP_AVC_GRP2 + 0x0c)
#define   D_SDET_SILENCE_LEVEL_DEC2             (DSP_AVC_GRP2 + 0x10)
#define   W_SDET_SILENCE_PERIOD_DEC2            (DSP_AVC_GRP2 + 0x14)

/************** PTS/STC for first decoder ********************/
#define   DSP_STC_GRP2                  (39 * 32)

#define   D_1ST_PTS_PRS_PNT_DEC2                (DSP_STC_GRP2 + 0x00) // second decoder
#define   W_1ST_PTS_STCH_DEC2                   (DSP_STC_GRP2 + 0x04) // second decoder
#define   W_1ST_PTS_STCL_DEC2                   (DSP_STC_GRP2 + 0x06) // second decoder
#define   D_STC_DIFF_LO_DEC2                    (DSP_STC_GRP2 + 0x08) // second decoder
#define   D_STC_DIFF_HI_DEC2                    (DSP_STC_GRP2 + 0x0c) // second decoder
#define   D_STC_DIFF_WS_DEC2                    (DSP_STC_GRP2 + 0x10) // second decoder
#define   D_STC_THRESHOLD_DEC2                  (DSP_STC_GRP2 + 0x14) // second decoder

/************** 3D(Reverb) for second decoder ********************/
#define   DSP3D_GRP32                   (40 * 32)
#define   D_REVERBMEMADDR_DEC2                    (DSP3D_GRP32 + 0x00)    // second decoder
#define   B_REVERBFLAG_DEC2                        (DSP3D_GRP32 + 0x04)    // second decoder
#define   D_REVERBGAIN_DEC2                        (DSP3D_GRP32 + 0x08)    // second decoder
#define   B_REVERBBANK0_DEC2                    (DSP3D_GRP32 + 0x0C)    // second decoder
#define   B_REVERBBANK1_DEC2                    (DSP3D_GRP32 + 0x0D)    // second decoder
#define   B_REVERBBANK2_DEC2                    (DSP3D_GRP32 + 0x0E)    // second decoder
#define   B_REVERBBANK3_DEC2                    (DSP3D_GRP32 + 0x0F)    // second decoder

/************** 3D(VSS/PL2) for second decoder **************/
#define   DSP3D_GRP42                   (41 * 32)
/* currently not support */
#define   B_VSURRFLAG_DEC2                        (DSP3D_GRP42 + 0x04)    // second decoder
#define   B_VSURRDELAYNUM_DEC2                    (DSP3D_GRP42 + 0x05)    // second decoder
#define   D_VSURRGAIN_DEC2                        (DSP3D_GRP42 + 0x08)    // second decoder
#define   D_VSURRWIDEGAIN_DEC2                    (DSP3D_GRP42 + 0x0C)    // second decoder

#define   SECONDARY_DECODER                (42 * 32)
//For new codec and secondary decoder

#define   REENCODER_GRP                 (43 * 32)
#define   B_ENC_SFREQ                           (REENCODER_GRP + 0x0)
#define   B_ENC_BITRATE                         (REENCODER_GRP + 0x1)
#define   B_ENC_CH_MODE                         (REENCODER_GRP + 0x2)
#define   B_ENC_BIT_LENGTH                      (REENCODER_GRP + 0x3)
#define   W_ENC_SOFT_REMAIN                     (REENCODER_GRP + 0x4)
#define   W_ENC_FLUSH_REMAIN                    (REENCODER_GRP + 0x6)
#define   B_ENC_SOFT_ORDER                      (REENCODER_GRP + 0x8)
#define   W_REENC_AC3_FLAG                      (REENCODER_GRP + 0x10)  //AC3
#define   B_REENC_DTS_AMODE                        (REENCODER_GRP + 0x12)  //DTS

#define   TRANSCODER_GRP                (44 * 32)
#define   B_TRANS_BITRATE                       (TRANSCODER_GRP + 0x0)
#define   B_TRANS_CH_MODE                       (TRANSCODER_GRP + 0x1)
#define   W_TRANS_MPEG_FLAG                     (TRANSCODER_GRP + 0x10)  //MP3

#define   MIC_GRP1                        (45 * 32)
#define   B_MPDOWNSAMPLE                        (MIC_GRP1 + 0x0)  //Input oversampling
#define   B_MPUPORDER                           (MIC_GRP1 + 0x1)  //Output level detection
#define   B_MPDOWNORDER                         (MIC_GRP1 + 0x2)
#define   B_MPFLAG                              (MIC_GRP1 + 0x3)  //Microphone flag
#define   B_MP_CHMIX                            (MIC_GRP1 + 0x4)  //Output channel mixing
#define   B_MICUPORDER                          (MIC_GRP1 + 0x5)  //Input level detection
#define   B_MICDOWNORDER                        (MIC_GRP1 + 0x6)
#define   B_INTERNAL_ADC                        (MIC_GRP1 + 0x7)  //Internal ADC input
#define   D_MPVOL1                              (MIC_GRP1 + 0x8)  //Input 1 volume
#define   D_MPVOL2                              (MIC_GRP1 + 0xC)  //Input 2 volume
#define   D_MPECHOVOL                           (MIC_GRP1 + 0x10) //Microphone echo vol
#define   D_MPDEPTH                             (MIC_GRP1 + 0x14) //Microphone echo depth
#define   W_MPDELAY                             (MIC_GRP1 + 0x18) //Microphone delay
#define   W_MIC_MUTE_BNK                        (MIC_GRP1 + 0x1A)

#define   MIC_GRP2                      (46 * 32)
#define   D_MPEQGAIN_DRY                        (MIC_GRP2 + 0)    //Microphone EQ
#define   D_MPEQGAIN_B1                         (MIC_GRP2 + 0x4)
#define   D_MPEQGAIN_B2                         (MIC_GRP2 + 0x8)
#define   D_MPEQGAIN_B3                         (MIC_GRP2 + 0xC)
#define   B_MP_REC_FLAG                         (MIC_GRP2 + 0x10) //Microphone recording
#define   D_MIC_IN_THRESHOLD                    (MIC_GRP2 + 0x14) //Microphone input threshold
#define   D_MP_OUT_THRESHOLD                    (MIC_GRP2 + 0x18) //Microphone output threshold

#define      MIC_GRP3                         (47 * 32)
#define   D_MP_CHORUS_MIX_R                     (MIC_GRP3 + 0x00)  //Mic Chrous
#define   D_MP_CHORUS_FB_R                      (MIC_GRP3 + 0x04)
#define   B_MP_CHORUS_DELAY                     (MIC_GRP3 + 0x08)
#define   B_MP_CHORUS_DEPTH                     (MIC_GRP3 + 0x09)
#define   B_MP_CHORUS_FREQ                      (MIC_GRP3 + 0x0A)
#define   B_MP_REVERB_FLAG                      (MIC_GRP3 + 0x0B)  //Mic Reverb
#define   B_MP_REVERB_BANK1                     (MIC_GRP3 + 0x0C)
#define   B_MP_REVERB_BANK2                     (MIC_GRP3 + 0x0D)
#define   B_MP_REVERB_BANK3                     (MIC_GRP3 + 0x0E)
#define   B_MP_REVERB_BANK4                     (MIC_GRP3 + 0x0F)
#define   D_MP_REVERB_GAIN                      (MIC_GRP3 + 0x10)
#define   W_MP_KEYSHIFT                         (MIC_GRP3 + 0x14)  //Mic Key shift
#define   W_MP_KEY_MIX                          (MIC_GRP3 + 0x16)

#define   DSP_KSCORE_GRP                (48 * 32)
#define   B_KSCORECONFIG                        (DSP_KSCORE_GRP + 0x00)  //Karaoke scoring
#define   B_KSCORE_MAX                          (DSP_KSCORE_GRP + 0x01)
#define   B_KSCORE_MIN                          (DSP_KSCORE_GRP + 0x02)
#define   B_KSCORE_SCORE                        (DSP_KSCORE_GRP + 0x03)
#define   W_KSCORE_HIT0                         (DSP_KSCORE_GRP + 0x04)
#define   W_KSCORE_HIT1                         (DSP_KSCORE_GRP + 0x06)
#define   W_KSCORE_MISS                         (DSP_KSCORE_GRP + 0x08)
#define   W_KSCORE_MICTHD                       (DSP_KSCORE_GRP + 0x0A)
#define   B_KSCORE_STRATEGY                     (DSP_KSCORE_GRP + 0x0C)
#define   B_KARAFLAG                            (DSP_KSCORE_GRP + 0x10)  //Karaoke function
#define   D_LRMIXRATIO                          (DSP_KSCORE_GRP + 0x14)

#define   DSP_VOICE_GRP                 (49 * 32)
#define   W_VOICE_LEVEL_TH                      (DSP_VOICE_GRP + 0)       //Voice detection
#define   W_VOICE_BANK_TH                       (DSP_VOICE_GRP + 0x2)
#define   W_VOICE_LEVEL                         (DSP_VOICE_GRP + 0x4)
#define   B_VOICE_FLAG                          (DSP_VOICE_GRP + 0x6)
#define   W_VOCDTKN                             (DSP_VOICE_GRP + 0x8)
#define   W_VOC_MUTEBANK                        (DSP_VOICE_GRP + 0xA)
#define   W_VOC_TH                              (DSP_VOICE_GRP + 0xC)

#define   DSP_ADDR_GRP                    (50 * 32)
#define   D_MPDELAYADDR                         (DSP_ADDR_GRP + 0x0)      // RISC*
#define   D_MEMBACKUPADDR                       (DSP_ADDR_GRP + 0x4)      // RISC*
#define   D_MICRECADDR                          (DSP_ADDR_GRP + 0xC)      // RISC*
#define   D_MICRECLENGTH                        (DSP_ADDR_GRP + 0x10)     // RISC*
#define   D_MIC_IO_ADR                          (DSP_ADDR_GRP + 0x14)     // RISC
#define   D_MIC_PITCH_ADR                       (DSP_ADDR_GRP + 0x18)     // RISC
#define   D_KEYPCM_ADDR                         (DSP_ADDR_GRP + 0x1C)     // RISC, MT1389

#define   DSP_ADDR_GRP2                  (51 * 32)
#define   D_UPSAMP_HIS_ADR                      (DSP_ADDR_GRP2 + 0x00)     // RISC, MT1389
#define   D_UPSAMP_COEF_ADR                     (DSP_ADDR_GRP2 + 0x04)     // RISC, MT1389
#define   D_DOLBY_HP_ADR                        (DSP_ADDR_GRP2 + 0x08)     // RISC, MT1389
#define   D_DNMX_BUF_ADR                        (DSP_ADDR_GRP2 + 0x0C)     // RISC, MT1389
#define   D_3D_BUF_ADDR                         (DSP_ADDR_GRP2 + 0x10)
#define   D_MP_CHORUS_ADR                       (DSP_ADDR_GRP2 + 0x14)
#define   D_PRIMARY_PTS_START_ADR                (DSP_ADDR_GRP2 + 0x18)
#define   D_PRIMARY_PTS_QUEUE_SIZE                (DSP_ADDR_GRP2 + 0x1C)

#define   DSP_INFO_GRP1                  (52 * 32)

#define   DSP_INFO_GRP2                  (53 * 32)
#define   D_MIC_LEVEL                           (DSP_INFO_GRP2 + 0)       // RISC
#define   B_MIC_LEVELMODE                       (DSP_INFO_GRP2 + 0x4)     // RISC
#define   B_MIC_REC_STATE                       (DSP_INFO_GRP2 + 0x6)     // RISC (0~0x80; 0x80 means rec buffer full)
#define   B_MIC_PLAY_STATE                      (DSP_INFO_GRP2 + 0x7)     // RISC
#define   D_MIC_REC_LENGTH                      (DSP_INFO_GRP2 + 0x8)     // RISC (DWRD Length) compared to D_MICRECLENGTH)
#define   D_MIC_PLAY_LENGTH                     (DSP_INFO_GRP2 + 0xC)     // RISC

#define   DSP_DEC1_GRP                  (54 * 32)
#define   D_VOL_DEC1                            (DSP_DEC1_GRP + 0x00)      // decoeder1's volume
#define   D_TEST_TONE_CHANNEL                   (DSP_DEC1_GRP + 0x04)      // for test tone channel setting
#define   B_MAX_SPK_CHECK_SPEAKER               (DSP_DEC1_GRP + 0x08)      // max speaker config: check speaker setting
#define   B_MAX_SPK_CHECK_HDMI                  (DSP_DEC1_GRP + 0x09)      // max speaker config: check HDMI sink capability
#define   B_FORCE_2CH_DOWNMIX                   (DSP_DEC1_GRP + 0x0A)      // special downmix to 2ch mode

/********************** post process used *********************************/
#define   DSP_POST_PROCESS_GRP1         (55 * 32)
#define   D_SE_MP3BOOSTER_FLAG                  (DSP_POST_PROCESS_GRP1 + 0x00)   // (aud_se_mp3booster)
#define   D_SE_MP3BOOSTER_COEF_ADDR             (DSP_POST_PROCESS_GRP1 + 0x04)
#define   D_SE_DYNAMICBASS_FLAG                 (DSP_POST_PROCESS_GRP1 + 0x08)   // (aud_se_dbass)
#define   D_SE_DYNAMICBASS_COEF_ADDR            (DSP_POST_PROCESS_GRP1 + 0x0C)
#define   D_SE_SWAP_FLAG                        (DSP_POST_PROCESS_GRP1 + 0x10)   // (aud_se_swap)
#define   D_SE_SWAP_COEF_ADDR                   (DSP_POST_PROCESS_GRP1 + 0x14)
#define   D_SE_SWAP_SPEAKER_CFG                 (DSP_POST_PROCESS_GRP1 + 0x18)
#define   B_SE_BASS_MANAGEMENT_MODE             (DSP_POST_PROCESS_GRP1 + 0x1C)   //0:LFE mode; 1:LFE+Main mode

//-----------------
#define   DVD_AUDIO_GRP                 (56 * 32)
#define   B_DVD_AUDIO_DEEMPHASIS                (DVD_AUDIO_GRP + 0x01) //DVD AUDIO EMPHASIS FLAGS
#define   B_DVD_AUDIO_CCI_INFO                  (DVD_AUDIO_GRP + 0x02) //DVD AUDIO COPY PERMISSION
#define   B_DVD_AUDIO_STEREO_PROHIBIT           (DVD_AUDIO_GRP + 0x03) //DVD AUDIO STEREO PLAYBACK MODE
#define   B_DVD_AUDIO_ATS_DM_VALID              (DVD_AUDIO_GRP + 0x04) //DVD AUDIO ATS DM VALID OR NOT
#define   B_LPE_DVDA_ON                         (DVD_AUDIO_GRP + 0x05) //Define DVDA stream

//---------------------------------------------------
//*****************for PHI 2010 Project used*******************************//
#define   PHI_MODULE_GRP                (57 * 32)
#define   B_PHI_LOGO_TRUEHD                     (PHI_MODULE_GRP + 0x00) //PHI LOGO TRUEHD
#define   B_PHI_LOGO_DD_PLUS                    (PHI_MODULE_GRP + 0x01) //PHI LOGO DD+
#define   B_PHI_LOGO_DD                         (PHI_MODULE_GRP + 0x02) //PHI LOGO DD
#define   B_PHI_LOGO_DTS20                        (PHI_MODULE_GRP + 0x03) //PHI LOGO DTS20
#define   B_PHI_LOGO_DTS_ADO                    (PHI_MODULE_GRP + 0x04) //PHI LOGO DTS_ADO
#define   B_PHI_LOGO_DTS_HD_MA                  (PHI_MODULE_GRP + 0x05) //PHI LOGO DTS_HD_MA
#define   B_PHI_LOGO_DTS_HD_MA_ESS              (PHI_MODULE_GRP + 0x06) //PHI LOGO DTS_HD_MA_ESS
#define  B_PHI_AOUT_STEREO_CHANNEL              (PHI_MODULE_GRP + 0x07) //PHI AOUT STEREO CHANNEL
#define  B_PHI_AOUT_MULTI_CHANNEL               (PHI_MODULE_GRP + 0x08) //PHI AOUT MULTI CHANNEL
#define B_PHI_DDCO                              (PHI_MODULE_GRP + 0x09) //PHI  DDCO
#define  B_PHI_BMANAGEMENT_STEREO_CHANNEL       (PHI_MODULE_GRP + 0x0a) //PHI BMANAGEMENT STEREO CHANNEL
#define  B_PHI_BMANAGEMENT_MULTI_CHANNEL        (PHI_MODULE_GRP + 0x0b) //PHI BMANAGEMENT MULTI CHANNEL
#define  B_PHI_PROLOGICII                       (PHI_MODULE_GRP + 0x0c) //PHI PROLOGICII
#define  B_PHI_SACD                             (PHI_MODULE_GRP + 0x0d) //PHI sacd
#define  B_PHI_AAC                              (PHI_MODULE_GRP + 0x0e) //PHI aac
#define  B_PHI_NEO6                             (PHI_MODULE_GRP + 0x0f) //PHI neo6
#define  B_DIVERSITY_AACUSED_ONLY               (PHI_MODULE_GRP + 0x10) // aacplus or aac
#define B_PHI_HDMI_DD_OUTPUT_2                  (PHI_MODULE_GRP + 0x10) //PHI HDMI DD OUTPUT 2
#define B_PHI_HDMI_DD_OUTPUT_5_1                (PHI_MODULE_GRP + 0x11) //PHI HDMI DD OUTPUT 5_1
#define B_PHI_HDMI_DD_OUTPUT_7_1                (PHI_MODULE_GRP + 0x12) //PHI HDMI DD OUTPUT 7_1
#define B_PHI_HDMI_DTS_OUTPUT_2                 (PHI_MODULE_GRP + 0x13) //PHI HDMI DTS OUTPUT 2
#define B_PHI_HDMI_DTS_OUTPUT_5_1               (PHI_MODULE_GRP + 0x14) //PHI HDMI DTS OUTPUT 5_1
#define B_PHI_HDMI_DTS_OUTPUT_7_1               (PHI_MODULE_GRP + 0x15) //PHI HDMI DTS OUTPUT 7_1
#define B_PHI_HDMI_OTHER_CODEC_OUTPUT_2         (PHI_MODULE_GRP + 0x16) //PHI HDMI OTHER CODEC OUTPUT 2
#define B_PHI_HDMI_OTHER_CODEC_OUTPUT_5_1       (PHI_MODULE_GRP + 0x17) //PHI HDMI OTHER CODEC OUTPUT 5_1
#define B_PHI_HDMI_OTHER_CODEC_OUTPUT_7_1       (PHI_MODULE_GRP + 0x18) //PHI HDMI OTHER CODEC OUTPUT 7_1
#define B_PHI_LOGO_DTS_DIGI_SURR                (PHI_MODULE_GRP + 0x19)
#define B_CD_OUTPUT_MODE                        (PHI_MODULE_GRP + 0x1A)

/*
bit 0 : CDDA_OUTPUT_PCM(0)/CDDA_OUTPUT_RAW(1)
bit 1:  HDCD_OUTPUT_PCM(0)/HDCD_OUTPUT_RAW(1)
*/
#define   MUTE_CTRL_GRP                 (58 * 32)
#define   W_MUTE_GLOBAL_SMOOTH                  (MUTE_CTRL_GRP + 0x00)  // global smooth mute
#define   W_MUTE_RESERVED                       (MUTE_CTRL_GRP + 0x02)
#define   W_MUTE_SPDIF_PCM                      (MUTE_CTRL_GRP + 0x04)  // mute SPDIF PCM by register
#define   W_MUTE_SPDIF_RAW                      (MUTE_CTRL_GRP + 0x06)  // mute SPDIF RAW by IEC flag
#define   W_MUTE_HDMI_PCM                       (MUTE_CTRL_GRP + 0x08)  // mute HDMI PCM by trim
#define   W_MUTE_HDMI_RAW                       (MUTE_CTRL_GRP + 0x0A)  // mute HDMI RAW by IEC2 flag
#define   W_MUTE_MULTI_CH                       (MUTE_CTRL_GRP + 0x0C)  // mute Analog multi-ch by trim
#define   W_MUTE_DNMX_2CH                       (MUTE_CTRL_GRP + 0x0E)  // mute Analog ch9/ch10 by trim

//---------------------------------------------------
//*****************for diversity used*******************************//
#define   DIVERSITY_TBL_GRP             (59 * 32)
#define   B_DIV_TBL_LOGO_DOLBY                  (DIVERSITY_TBL_GRP + 0x00) //Dolby LOGOs
/*----------------------------------------------------------------------------
B_LOGO_DOLBY   1:Support;  0:Not Support
Bit0:Dolby Digital
Bit1:Dolby DD+
Bit2:Dolby TrueHD
----------------------------------------------------------------------------*/
#define   B_DIV_TBL_LOGO_DTS                    (DIVERSITY_TBL_GRP + 0x01) //DTS LOGO
/*----------------------------------------------------------------------------
B_LOGO_DTS     1:Support;  0:Not Support
Bit0:DTS2.0
Bit1:DTS Digital Surround
Bit2:DTS-HD MA Essential
Bit3:DTS-HD MA
Bit4:DTS ADO
----------------------------------------------------------------------------*/
#define  B_DIV_TBL_ANALOG_OUTPUT_CHANNEL        (DIVERSITY_TBL_GRP + 0x02) //analog channel num config
/*----------------------------------------------------------------------------
B_ANALOG_OUTPUT_CHANNEL
Bit[1:0]:analog channel num
    0:Un-init     1:2-ch;    2:Multi-channel
----------------------------------------------------------------------------*/
#define  B_DIV_TBL_HDMI_OUTPUT_CHANNEL          (DIVERSITY_TBL_GRP + 0x03) //hdmi channel num config
/*----------------------------------------------------------------------------
B_HDMI_OUTPUT_CHANNEL
Bit[1:0]:Dolby HDMI output channel num
    0:Un-init     1: 2ch;    2: 5.1ch;    3: 7.1ch
Bit[3:2]:DTS HDMI output channel num
    0:Un-init     1: 2ch;    2: 5.1ch;    3: 7.1ch
Bit[5:4]:Others HDMI output channel num
    0:Un-init     1: 2ch;    2: 5.1ch;    3: 7.1ch
----------------------------------------------------------------------------*/
#define  B_DIV_TBL_POST_PROCESS                 (DIVERSITY_TBL_GRP + 0x04) //Post-processing select
/*----------------------------------------------------------------------------
B_POST_PROCESS
Bit[1:0]:bass management channel num
    0:Un-init     1:2-ch;  2:Multi-channel
Bit2:PL2          1:Support;  0:Not Support
Bit3:Neo6         1:Support;  0:Not Support
----------------------------------------------------------------------------*/
#define  B_DIV_TBL_CODEC_SUPPORT                (DIVERSITY_TBL_GRP + 0x05) //Other codec
/*----------------------------------------------------------------------------
B_CODEC_SUPPORT   1:Support;  0:Not Support
Bit0:DDCO
Bit1:SACD
Bit2:AAC
Bit3:AAC Decode used decode or AAC PLUS
----------------------------------------------------------------------------*/

//---------------------------------------------------
//*****************Codec digital output info*******************************//
#define   CODEC_DIGI_OUT_INFO_TBL_GRP   (60 * 32)
#define   B_DISC_HAS_SA_IA                      (CODEC_DIGI_OUT_INFO_TBL_GRP + 0x00) //SA/IA existance
#define   B_DOLBY_DIGI_OUT_FMT                  (CODEC_DIGI_OUT_INFO_TBL_GRP + 0x01) //Dolby
// 0 - unknown
// 1 - bitstream (RAW or Reencode)
// 2 - PCM
#define   B_DTS_DIGI_OUT_FMT                    (CODEC_DIGI_OUT_INFO_TBL_GRP + 0x02) //DTS
// 0 - unknown
// 1 - bitstream (RAW or Reencode)
// 2 - PCM
#define   B_UI_MIXER_SETTING                    (CODEC_DIGI_OUT_INFO_TBL_GRP + 0x03)
#define   B_UI_HDMI_IS_OFF                      (CODEC_DIGI_OUT_INFO_TBL_GRP + 0x04)
// FALSE - ON
// TRUE - OFF
//---------------------------------

#define   CODEC_DUAL_PLAY_CONTROL_GRP   (61 * 32)
#define   B_FRONT_AOUT_MEDIA_TYPE               (CODEC_DUAL_PLAY_CONTROL_GRP + 0x00)
#define   B_REAR_AOUT_MEDIA_TYPE                (CODEC_DUAL_PLAY_CONTROL_GRP + 0x01)
#define   B_AUDIN_INPUT_TYPE                    (CODEC_DUAL_PLAY_CONTROL_GRP + 0x02)
#define   W_MEDIA_TYPE                          (CODEC_DUAL_PLAY_CONTROL_GRP + 0x04)


/********************** Rear Vol used *********************************/
#define  VOL_REAR_GRP                   (62 * 32)
#define   D_VOL_REAR                            (VOL_REAR_GRP + 0x00) //REAR VOL

#define   DSP_DEBUG_GRP                 (63 * 32)
#define   B_DSP_DEBUG_1                         (DSP_DEBUG_GRP + 0x00)

#if 0
// **************** emulation only *************************************
#define   DSPSPECTRUM_GRP0                  ((DSP_GRP_MAX + 0) * 32)
#define   W_SPEC0                               (DSPSPECTRUM_GRP0 + 0x00) // RISC
#define   W_SPEC1                               (DSPSPECTRUM_GRP0 + 0x02) // RISC
#define   W_SPEC2                               (DSPSPECTRUM_GRP0 + 0x04) // RISC
#define   W_SPEC3                               (DSPSPECTRUM_GRP0 + 0x06) // RISC
#define   W_SPEC4                               (DSPSPECTRUM_GRP0 + 0x08) // RISC
#define   W_SPEC5                               (DSPSPECTRUM_GRP0 + 0x0A) // RISC
#define   W_SPEC6                               (DSPSPECTRUM_GRP0 + 0x0C) // RISC
#define   W_SPEC7                               (DSPSPECTRUM_GRP0 + 0x0E) // RISC
#define   W_SPEC8                               (DSPSPECTRUM_GRP1 + 0x10) // RISC
#define   W_SPEC9                               (DSPSPECTRUM_GRP1 + 0x12) // RISC
#define   W_SPEC10                              (DSPSPECTRUM_GRP1 + 0x14) // RISC
#define   W_SPEC11                              (DSPSPECTRUM_GRP1 + 0x16) // RISC
#define   W_SPEC12                              (DSPSPECTRUM_GRP1 + 0x18) // RISC
#define   W_SPEC13                              (DSPSPECTRUM_GRP1 + 0x1A) // RISC
#define   W_SPEC14                              (DSPSPECTRUM_GRP1 + 0x1C) // RISC
#define   W_SPEC15                              (DSPSPECTRUM_GRP1 + 0x1E) // RISC

#define   DSPSPECTRUM_GRP1                  ((DSP_GRP_MAX + 1) * 32)
#define   W_SPEC16                              (DSPSPECTRUM_GRP1 + 0x00) // RISC
#define   W_SPEC17                              (DSPSPECTRUM_GRP1 + 0x02) // RISC
#define   W_SPEC18                              (DSPSPECTRUM_GRP1 + 0x04) // RISC
#define   W_SPEC19                              (DSPSPECTRUM_GRP1 + 0x06) // RISC
#define   W_SPEC20                              (DSPSPECTRUM_GRP1 + 0x08) // RISC
#define   W_SPEC21                              (DSPSPECTRUM_GRP1 + 0x0A) // RISC
#define   W_SPEC22                              (DSPSPECTRUM_GRP1 + 0x0C) // RISC
#define   W_SPEC23                              (DSPSPECTRUM_GRP1 + 0x0E) // RISC
#define   W_SPEC24                              (DSPSPECTRUM_GRP3 + 0x10) // RISC
#define   W_SPEC25                              (DSPSPECTRUM_GRP3 + 0x12) // RISC
#define   W_SPEC26                              (DSPSPECTRUM_GRP3 + 0x14) // RISC
#define   W_SPEC27                              (DSPSPECTRUM_GRP3 + 0x16) // RISC
#define   W_SPEC28                              (DSPSPECTRUM_GRP3 + 0x18) // RISC
#define   W_SPEC29                              (DSPSPECTRUM_GRP3 + 0x1A) // RISC
#define   W_SPEC30                              (DSPSPECTRUM_GRP3 + 0x1C) // RISC
#define   W_SPEC31                              (DSPSPECTRUM_GRP3 + 0x1E) // RISC

#define   DSP_EMU_GRP1                      ((DSP_GRP_MAX + 2) * 32)
#define   D_STC                                 (DSP_EMU_GRP1 + 0x00) // RISC
#define   D_PTS_SIZE                            (DSP_EMU_GRP1 + 0x04) // RISC
#define   W_STC_SETTING                         (DSP_EMU_GRP1 + 0x08) // RISC
#define   D_STC_DEC2                            (DSP_EMU_GRP1 + 0x0A) // RISC
#define   D_PTS_SIZE_DEC2                       (DSP_EMU_GRP1 + 0x0E) // RISC
#define   W_STC_SETTING_DEC2                    (DSP_EMU_GRP1 + 0x12) // RISC

#define   DSP_CH_VOL_GRP                    ((DSP_GRP_MAX + 3) * 32)
#define   D_CH_L_VOL                            (DSP_CH_VOL_GRP + 0x00)   // RISC
#define   D_CH_R_VOL                            (DSP_CH_VOL_GRP + 0x04)   // RISC
#define   D_CH_LS_VOL                           (DSP_CH_VOL_GRP + 0x08)   // RISC
#define   D_CH_RS_VOL                           (DSP_CH_VOL_GRP + 0x0C)   // RISC
#define   D_CH_C_VOL                            (DSP_CH_VOL_GRP + 0x10)   // RISC
#define   D_CH_SUB_VOL                          (DSP_CH_VOL_GRP + 0x14)   // RISC
#define   D_CH7_VOL                             (DSP_CH_VOL_GRP + 0x18)   // RISC
#define   D_CH8_VOL                             (DSP_CH_VOL_GRP + 0x1c)   // RISC

#define   DSP_CH_VOL_GRP2                   ((DSP_GRP_MAX + 4) * 32)
#define   D_CH9_VOL                             (DSP_CH_VOL_GRP2 + 0x00)  // RISC
#define   D_CH10_VOL                            (DSP_CH_VOL_GRP2 + 0x04)  // RISC
#define   D_CH_L_VOL_DEC2                       (DSP_CH_VOL_GRP2 + 0x08)  // RISC
#define   D_CH_R_VOL_DEC2                       (DSP_CH_VOL_GRP2 + 0x0C)  // RISC
#define   D_CH_VOLUME_TOTAL                     (DSP_CH_VOL_GRP2 + 0x10)  // RISC
#define   D_CH_VOLUME_TOTAL_DEC2                (DSP_CH_VOL_GRP2 + 0x14)  // RISC
//*****************************************************************
#endif
#endif /* _DSP_SHM_H_ */


