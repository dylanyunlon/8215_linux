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

#ifndef _X_AUDMHL_DEF_H_
#define _X_AUDMHL_DEF_H_

#define AUDMHL_MW_EVENT    TEXT("AUDMHL_MW_EVENT")

typedef struct _HDMI_RX_AUDIO_CHSTS {
    __u8 rev :1;
    __u8 ISLPCM :1;     //0:LPCM 1:non-PCM
    __u8 CopyRight : 1;
    __u8 AdditionFormatInfo :3;
    __u8 ChannelStatusMode :2;
    __u8 CategoryCode;
    __u8 SourceNumber :4;
    __u8 ChannelNumber :4;
    __u8 SamplingFreq :4;
    __u8 ClockAccuary :2;
    __u8 rev2 : 2;
    __u8 WorldLen : 4;
    __u8 OriginalSamplingFreq :4;
}HDMI_RX_AUDIO_CHSTS;

typedef struct _HDMI_RX_PCM_INFO_T
{
    __u8 AudioChannelCount:3;   //HDMI_RX_AUDIO_CHANNEL_COUNT_T
    __u8 DM_INH:1;              // 1:Down mix Prohibites, 0:Permitted down mix
    __u8 AudioCodingType:4;     //see HDMI_RX_AUDIO_CODING_TYPE_T
    __u8 LevelShiftValue:4;     //HDMI_RX_AUD_LEVEL_TYPE_T
    __u8 SampleSize:2;          //HDMI_RX_SAMPLE_SIZE_TYPE_T
    __u8 SpeakerPlacement;      //HDMI_RX_SPEAKER_ALLOCATE_T
}HDMI_RX_PCM_INFO_T;

typedef enum
{
    HDMI_RX_NONE =0 ,
    HDMI_RX_PCM ,
    HDMI_RX_SD_RAW ,
    HDMI_RX_HBR ,
    HDMI_RX_DSD ,
    HDMI_RX_192k_RAW
}HDMI_RX_AUDIO_FORMAT_T;

/*-----------------------------------------------------------------------------
                    Notify function (AUDIN to SCOM)
 ----------------------------------------------------------------------------*/
 /* Notify conditions */
typedef enum
{
    AUDIN_CHG_PAUSE_STATUS = 0, //  pause status
    AUDIN_CHG_SPDIFIN_INT_SIGNAL, // UnLock
    AUDIN_CHG_SPDIFIN_INT_INDET, // Input pin status 
    AUDIN_CHG_SPDIFIN_INT_FSCHG, // Fs calculation updated
    AUDIN_CHG_SPDIFIN_INT_MCLK, // MCLK info updated
    AUDIN_CHG_SPDIFIN_INPUT_SW, // Audio Input switch
    AUDIN_CHG_ON_OFF, // Audio Input On/Off
    AUDIN_CHG_HDMIRX_INT,
    AUDIN_SPDIF_AUDIN_TYPE,    // PCM/RAW/DTSCD-16,DTSCD-14 
    AUDIN_SPDIF_RAW_DATA_TYPE,   //SPDIF In Audio Codec Type      // AUD_DRV_FMT_T
    AUDIN_CHG_INT_CHNOCHG,
    AUDIN_CHG_SPDIFIN_OEMPF, // Audio emphasis
    AUDIN_CHG_HDMI_RX_PCM_CHLAYOUT,
    AUDIN_SPDIF_HW_ERROR,
    AUDIN_TASK_DELETE = 0xFF
}   AUDIN_COND_T;

 /* Notify information */
typedef   struct  _AUDIN_INFO_T
{
    __u8 u1AudinPauseStatus;  // 1 : PAUSE  ; 0: PLAY
    __u8 u1AudinLockStatus;  // 1 : Unlock ;  0: Lock
    __u8 u1AudinChStatus;  // For each bit (0~6) 1: (RX0 ~ RX6) input data exists   ;  0: no data
    __u8 u1AudinSampleRate;  // 0~6 : Out of range ; 7~0xF : 32/44.1/48/64/88.2/96/128/176.4/192 KHz
    __u8 u1AudinSwitchOK; // 1 : Input switch OK ;  0: NG
    __u8 u1AudinOnOffOK; // 1 : Input On/Off OK ;  0: NG
    __u8 u1HdmiRxINT;
    __u8 u1SpdifAudinType;    // SPDIFIn Audio Type : PCM/RAW/DTSCD-16/DTSCD-14
    __u8 u1SpdifRawDataType; //  Codec Type  , AC3,DTS,MPEG,AAC
    __u8 u1AudinUSBNo;  // no. of usb channel
    HDMI_RX_AUDIO_FORMAT_T u1HDMIRxAudFmt ;  // HDMI Rx audio format 
    HDMI_RX_PCM_INFO_T u4HDMIIRxPCMInfo;        // HDMI Rx PCM info
    HDMI_RX_AUDIO_CHSTS u8HDMIRxAudCHSTS ; // HDMI Rx audio channel status
}AUDIN_INFO_T;

typedef enum 
{
    AUDMHL_CLOSE = 0,
    AUDMHL_OPEN,
    AUDMHL_START,
    AUDMHL_STOP
}AUDMHL_OPEN_CTRL;

typedef enum
{   
    CA_FL_FR =0,
    CA_LFE_FL_FR = 0x01,
    CA_FC_FL_FR = 0x02,
    CA_FC_LFE_FL_FR = 0x03, 
    CA_RC_FL_FR = 0x04, 
    CA_RC_LFE_FL_FR = 0x05, 
    CA_RC_FC_FL_FR = 0x06, 
    CA_RC_FC_LFE_FL_FR = 0x07, 
    CA_RR_RL_FL_FR = 0x08, 
    CA_RR_RL_LFE_FL_FR = 0x09, 
    CA_RR_RL_FC_FL_FR = 0x0A, 
    CA_RR_RL_FC_LFE_FL_FR = 0x0B, 
    CA_RC_RR_RL_FL_FR = 0x0C, 
    CA_RC_RR_RL_LFE_FL_FR = 0x0D, 
    CA_RC_RR_RL_FC_FL_FR = 0x0E, 
    CA_RC_RR_RL_FC_LFE_FL_FR = 0x0F, 
    CA_RRC_RLC_RR_RL_FL_FR = 0x10,  
    CA_RRC_RLC_RR_RL_LFE_FL_FR = 0x11,  
    CA_RRC_RLC_RR_RL_FC_FL_FR = 0x12,  
    CA_RRC_RLC_RR_RL_FC_LFE_FL_FR = 0x13,  
    CA_FRC_FLC_FL_FR = 0x14,
    CA_FRC_FLC_LFE_FL_FR = 0x15,
    CA_FRC_FLC_FC_FL_FR = 0x16,
    CA_FRC_FLC_FC_LFE_FL_FR = 0x17,
    CA_FRC_FLC_RC_FL_FR = 0x18,
    CA_FRC_FLC_RC_LFE_FL_FR = 0x19,
    CA_FRC_FLC_RC_FC_FL_FR = 0x1A,
    CA_FRC_FLC_RC_FC_LFE_FL_FR = 0x1B,
    CA_FRC_FLC_RR_RL_FL_FR = 0x1C,
    CA_FRC_FLC_RR_RL_LFE_FL_FR = 0x1D,
    CA_FRC_FLC_RR_RL_FC_FL_FR = 0x1E,
    CA_FRC_FLC_RR_RL_FC_LFE_FL_FR = 0x1F,
    CA_UNKNOWN  
} HDMI_RX_SPEAKER_ALLOCATE_T;

typedef enum
{
    SPDIFIN_OUT_RANGE = 0x00, //0x00~0x06 Freq out of range
    SPDIFIN_32K = 0x07,
    SPDIFIN_44K = 0x08,
    SPDIFIN_48K = 0x09,
    SPDIFIN_64K = 0x0A,
    SPDIFIN_88K = 0x0B,
    SPDIFIN_96K = 0x0C,
    SPDIFIN_128K = 0x0D,
    SPDIFIN_176K = 0x0E,
    SPDIFIN_192K = 0x0F
} SPDIFIN_FS;

//  SPDIF In Data Type : PCM/RAW/DTSCD-16/DTSCD-14
typedef enum
{   
    SPDIFIN_PCM= 0,         
    SPDIFIN_RAW,              
    SPDIFIN_DTSCD_16,   
    SPDIFIN_DTSCD_14    
} SPDIFIN_IN_FORMAT_T;

#endif
