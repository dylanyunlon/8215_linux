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

#ifndef _X_AVD_H_
#define _X_AVD_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "x_common.h"
#include "x_rm.h"
#include "x_drv_cb.h"
#include "x_memtype.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
/* Get operations */
#define AVD_GET_EDID_INFO          (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 0))//AVD will return HDMI_EDID_INFO_T structure EDID information
#define AVD_GET_SACD_SUPPORT_TYPE  (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 1))//AVD will return HDMI_EDID_INFO_T structure EDID information
#define AVD_GET_SRM_SIGNATURE_CHK  (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 2))//AVD will return AVD_GET_SRM_SIGNATURE_CHK_INF

/* Set operations */
#if UNIFORM_DRV_CALLBACK
#define AVD_SET_TYPE_NFY_FCT        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 0))//SCOM set notify function to AVD
#else
#define AVD_SET_TYPE_NFY_FCT       ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 0)) | RM_SET_TYPE_ARG_NO_REF) //SCOM set notify function to AVD
#endif
#define AVD_SET_TYPE_SRM_CHG_FCT        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 1))//SRM HDCP CPS service set SRM function to AVD
#define AVD_SET_TYPE_INFO        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)2)) //for 8530 re-encode flow
#define SET_PHILI_AVD_INFO         (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)3)) //for set avd info
#define SET_SO_NY_AVD_INFO         (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)4)) //for set avd info
#define AVD_SET_JPEG_PLAY_INFO     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)5)) //for set avd info, jpeg play
#define AVD_SET_GAMMA_SETTING_INFO     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)6)) //for set avd info, Gamma setting
#define AVD_SET_XVCOLOR_SETTING_INFO     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)7)) //for set avd info, xvColor
#define AVD_SET_TV_TYPE_INFO     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)8)) //for set avd info, TV TYPE
#define AVD_SET_EDID_DELAY_ENABLE  (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)9)) //for enable/disable avd to plus EDID delay to UI delay
#define AVD_SET_HDMI_ACP     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)10)) //sand an ACP packet to HDMI
#define AVD_SET_HDMI_AUDIO_MUTE_INFO     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)11)) //for set avd info, set HDMI audio Mute, see AVD_SET_HDMI_AUDIO_MUTE_SETTING
#define AVD_SET_HDMI_SPD     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)12)) //sand an SPD packet to HDMI
#define AVD_SET_HDMI_AVI_CN_TYPE     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)13)) //sand an SPD packet to HDMI


/*-----------------------------------------------------------------------------
                    EDID information
 ----------------------------------------------------------------------------*/
//This EDID_VIDEO_RES_T will define what kind of
//video resolution can be supported by sink.
typedef enum
{
  EDID_VIDEO_480P      = (1<< 0),
  EDID_VIDEO_720P60    = (1<< 1),
  EDID_VIDEO_1080I60   = (1<< 2),
  EDID_VIDEO_1080P60   = (1<< 3),
  EDID_VIDEO_480P_1440 = (1<< 4),
  EDID_VIDEO_480P_2880 = (1<< 5),
  EDID_VIDEO_480I      = (1<< 6),
  EDID_VIDEO_480I_1440 = (1<< 7),
  EDID_VIDEO_480I_2880 = (1<< 8),
  EDID_VIDEO_1080P30   = (1<< 9),
  EDID_VIDEO_576P      = (1<< 10),
  EDID_VIDEO_720P50    = (1<< 11),
  EDID_VIDEO_1080I50   = (1<< 12),
  EDID_VIDEO_1080P50   = (1<< 13),
  EDID_VIDEO_576P_1440 = (1<< 14),
  EDID_VIDEO_576P_2880 = (1<< 15),
  EDID_VIDEO_576I      = (1<< 16),
  EDID_VIDEO_576I_1440 = (1<< 17),
  EDID_VIDEO_576I_2880 = (1<< 18),
  EDID_VIDEO_1080P25   = (1<< 19),
  EDID_VIDEO_1080P24   = (1<< 20),
  EDID_VIDEO_1080P23_976   = (1<< 21),
  EDID_VIDEO_1080P29_97   = (1<< 22),
}  EDID_VIDEO_RES_T;


//This EDID_VIDEO_COLORIMETRY_T will define what kind of YCBCR
//can be supported by sink.
//And each bit also defines the colorimetry data block of EDID.
typedef enum
{
  EDID_COLOR_SPACE_YCBCR_444 = (1<<0),
  EDID_COLOR_SPACE_YCBCR_422 = (1<<1),
  EDID_COLOR_SPACE_XV_YCC709 = (1<<2),
  EDID_COLOR_SPACE_XV_YCC601 = (1<<3),
  EDID_COLOR_SPACE_METADATA0 = (1<<4),
  EDID_COLOR_SPACE_METADATA1 = (1<<5),
  EDID_COLOR_SPACE_METADATA2 = (1<<6),
  EDID_COLOR_SPACE_RGB       = (1<<7),
}  EDID_VIDEO_COLORIMETRY_T;


//EDID_AUDIO_DECODER_T define what kind of audio decoder
//can be supported by sink.
typedef enum
{
  EDID_AUDIO_DEC_LPCM             =  (1<<0),
  EDID_AUDIO_DEC_AC3              =  (1<<1),
  EDID_AUDIO_DEC_MPEG1            =  (1<<2),
  EDID_AUDIO_DEC_MP3              =  (1<<3),
  EDID_AUDIO_DEC_MPEG2            =  (1<<4),
  EDID_AUDIO_DEC_AAC              =  (1<<5),
  EDID_AUDIO_DEC_DTS              =  (1<<6),
  EDID_AUDIO_DEC_ATRAC            =  (1<<7),
  EDID_SINK_AUDIO_DEC_DSD         =  (1<<8),
  EDID_SINK_AUDIO_DEC_DOLBY_PLUS  =  (1<<9),
  EDID_SINK_AUDIO_DEC_DTS_HD      =  (1<<10),
  EDID_SINK_AUDIO_DEC_MAT_MLP     =  (1<<11),
  EDID_SINK_AUDIO_DEC_DST         =  (1<<12),
  EDID_SINK_AUDIO_DEC_WMA         =  (1<<13)
} EDID_AUDIO_DECODER_T;


//Deep color bit supported by Sink
typedef enum
{
  EDID_NO_DEEP_COLOR = 0,
  EDID_DEEP_COLOR_10_BIT = (1<<0),
  EDID_DEEP_COLOR_12_BIT = (1<<1),
  EDID_DEEP_COLOR_16_BIT = (1<<2)
} EDID_DEEP_COLOR_T;



typedef enum
{
  EDID_A_FMT_CH_32k_2CH = (1<<0),
  EDID_A_FMT_CH_44k_2CH = (1<<1),
  EDID_A_FMT_CH_48k_2CH = (1<<2),
  EDID_A_FMT_CH_88k_2CH = (1<<3),
  EDID_A_FMT_CH_96k_2CH = (1<<4),
  EDID_A_FMT_CH_176k_2CH = (1<<5),
  EDID_A_FMT_CH_192k_2CH = (1<<6),
  EDID_A_FMT_CH_32k_6CH = (1<<8),
  EDID_A_FMT_CH_44k_6CH = (1<<9),
  EDID_A_FMT_CH_48k_6CH = (1<<10),
  EDID_A_FMT_CH_88k_6CH = (1<<11),
  EDID_A_FMT_CH_96k_6CH = (1<<12),
  EDID_A_FMT_CH_176k_6CH = (1<<13),
  EDID_A_FMT_CH_192k_6CH = (1<<14),
  EDID_A_FMT_CH_32k_8CH = (1<<16),
  EDID_A_FMT_CH_44k_8CH = (1<<17),
  EDID_A_FMT_CH_48k_8CH = (1<<18),
  EDID_A_FMT_CH_88k_8CH = (1<<19),
  EDID_A_FMT_CH_96k_8CH = (1<<20),
  EDID_A_FMT_CH_176k_8CH = (1<<21),
  EDID_A_FMT_CH_192k_8CH = (1<<22)

} EDID_A_FMT_CH_TYPE;



typedef enum
{
  EDID_A_FMT_CH_32k_3CH = (1<<0),
  EDID_A_FMT_CH_44k_3CH = (1<<1),
  EDID_A_FMT_CH_48k_3CH = (1<<2),
  EDID_A_FMT_CH_88k_3CH = (1<<3),
  EDID_A_FMT_CH_96k_3CH = (1<<4),
  EDID_A_FMT_CH_176k_3CH = (1<<5),
  EDID_A_FMT_CH_192k_3CH = (1<<6),
  EDID_A_FMT_CH_32k_4CH = (1<<8),
  EDID_A_FMT_CH_44k_4CH = (1<<9),
  EDID_A_FMT_CH_48k_4CH = (1<<10),
  EDID_A_FMT_CH_88k_4CH = (1<<11),
  EDID_A_FMT_CH_96k_4CH = (1<<12),
  EDID_A_FMT_CH_176k_4CH = (1<<13),
  EDID_A_FMT_CH_192k_4CH = (1<<14),
  EDID_A_FMT_CH_32k_5CH = (1<<16),
  EDID_A_FMT_CH_44k_5CH = (1<<17),
  EDID_A_FMT_CH_48k_5CH = (1<<18),
  EDID_A_FMT_CH_88k_5CH = (1<<19),
  EDID_A_FMT_CH_96k_5CH = (1<<20),
  EDID_A_FMT_CH_176k_5CH = (1<<21),
  EDID_A_FMT_CH_192k_5CH = (1<<22),
  EDID_A_FMT_CH_32k_7CH = (1<<24),
  EDID_A_FMT_CH_44k_7CH = (1<<25),
  EDID_A_FMT_CH_48k_7CH = (1<<26),
  EDID_A_FMT_CH_88k_7CH = (1<<27),
  EDID_A_FMT_CH_96k_7CH = (1<<28),
  EDID_A_FMT_CH_176k_7CH = (1<<29),
  EDID_A_FMT_CH_192k_7CH = (1<<30)

} EDID_A_FMT_CH_TYPE1;


typedef enum
{
  HDMI_OUT_DVI_MODE = 0x01,
  HDMI_OUT_HDMI_MODE = 0x02,
  HDMI_OUT_DISABLE_MODE = 0xff
} HDMI_OUT_MODE;

//For AVD Audio interface

typedef enum
{
  AVD_HDMI_OUT_PRI = 0,
  AVD_COAXIAL_OUT_PRI = 1,
  AVD_MULTI_CH_ANALOG_OUT = 2,
  AVD_STEREO_ANALOG_OUT = 3,

} AVD_AUD_OUT_PRIORITY;


typedef enum
{
  AVD_BD_AUD_MIX = 0,
  AVD_BD_AUD_DIRECT = 1

} AVD_BD_AUD_SETTING;

typedef enum
{
  AVD_COAXIAL_DOLBY_DOWNMIX_PCM = 0,
  AVD_COAXIAL_DOLBY_DIGITAL = 1,

} AVD_COAXIAL_DOLBY_DIGITAL_SETTING;

typedef enum
{
  AVD_COAXIAL_DTS_DOWNMIX_PCM = 0,
  AVD_COAXIAL_DTS_RAW = 1,

} AVD_COAXIAL_DTS_SETTING;


typedef  enum
{
   AVD_UI_HDMI_AUDIO_PCM = 0, //PCM (Multi/Stereo channel)
   AVD_UI_HDMI_AUDIO_AUTO , //Raw or PCM
   AVD_UI_HDMI_AUDIO_REENCODE,
   AVD_UI_HDMI_AUDIO_OFF,
   AVD_UI_HDMI_AUDIO_PCM_2CH,//PCM 2 channel


} AVD_UI_HDMI_AUDIO_SETTING;


typedef enum
{
   AVD_UI_GAMMA_STANDARD = 0,
   AVD_UI_GAMMA_BRIGHTER ,
   AVD_UI_GAMMA_THEATRE,

} AVD_UI_GAMMA_MODE_SETTING;


typedef  enum
{
   AVD_UI_XVCOLOR_AUTO = 0,
   AVD_UI_XVCOLOR_OFF ,

} AVD_UI_XVCOLOR_MODE_SETTING;


typedef  enum
{
   AVD_UI_TV_TYPE_4_3 = 0,
   AVD_UI_TV_TYPE_16_9 ,

} AVD_UI_TV_TYPE_SETTING;


typedef enum
{
    /** @brief Disc play type information */
    //Please don't change it's order  rashly, because it relates with
    //the disc identification mapping.
    AVD_DISC_BDAV=0,            ///< BDAV                         BD Disc
    AVD_DISC_BDMV,            ///< BDMV                         BD Disc
    AVD_DISC_DVDVID,          ///< dvd video                    DVD   Disc
    AVD_DISC_DVDAUD,          ///< dvd audio                    DVD   Disc
    AVD_DISC_DVDMVR,          ///< dvd mvr                      DVD VR   Disc
    AVD_DISC_DVDPVR,          ///< dvd pvr                      DVD VR   Disc
    AVD_DISC_MOVIE_DIVX,           ///<  DIVX codec video file
    AVD_DISC_DTSCD,
    AVD_DISC_FORCE_REENCODE,
    AVD_DISC_FACTORY_MODE,
    AVD_DISC_OTHER,          ///< Other media
    AVD_DISC_BIVL,          ///< BIVL
}AVD_DISC_TYPE;

typedef enum
{
    AVD_DIGITAL_OUT_AUDIOPHILE=0,
    AVD_DIGITAL_OUT_AUTO,
    AVD_DIGITAL_OUT_PCM,
}AVD_DIGITAL_OUTPUT_TYPE;

typedef enum
{
  AVD_DSD_OFF = 0,
  AVD_DSD_ON = 1,

} AVD_DSD_SETTING;


typedef enum
{
  SPEAKER = 0,
  SPEAKER_AND_HDMI = 1,
  HDMI =2

} BD_DVD_AUDIO_OUTPUT_SETTING;


typedef enum
{
  AVD_AAC_PCM = 0,
  AVD_AAC_RAW = 1,

} AVD_AAC_SETTING;

typedef enum
{
  AVD_AUDIO_FILE = 0,
  AVD_VIDEO_FILE = 1,

} AVD_FILE_TYPE;


typedef enum
{
  AVD_SET_HDMI_AUDIO_UNMUTE = 0,
  AVD_SET_HDMI_AUDIO_MUTE = 1,

} AVD_SET_HDMI_AUDIO_MUTE_SETTING;


typedef enum
{
  AVD_CHK_FORMAL_SIGNATURE =0,
  AVD_NO_CHK_DVD_SRM_SIGNATURE = 1,
  AVD_NO_CHK_BD_SRM_SIGNATURE = 2,
  AVD_NO_CHK_SACD_SRM_SIGNATURE = 3

} AVD_GET_SRM_SIGNATURE_CHK_INF;


typedef   struct  _HDMI_EDID_INFO_T
{
  UINT32 ui4_ntsc_resolution;//use EDID_VIDEO_RES_T, there are many resolution
  UINT32 ui4_pal_resolution;// use EDID_VIDEO_RES_T

  UINT32 ui4_sink_native_ntsc_resolution;//use EDID_VIDEO_RES_T, only one NTSC resolution, Zero means none native NTSC resolution is avaiable

  UINT32 ui4_sink_native_pal_resolution; //use EDID_VIDEO_RES_T, only one resolution, Zero means none native PAL resolution is avaiable

  UINT32 ui4_sink_cea_ntsc_resolution;//use EDID_VIDEO_RES_T
  UINT32 ui4_sink_cea_pal_resolution;//use EDID_VIDEO_RES_T

  UINT32 ui4_sink_dtd_ntsc_resolution;//use EDID_VIDEO_RES_T
  UINT32 ui4_sink_dtd_pal_resolution;//use EDID_VIDEO_RES_T

  UINT32 ui4_sink_1st_dtd_ntsc_resolution;//use EDID_VIDEO_RES_T
  UINT32 ui4_sink_1st_dtd_pal_resolution;//use EDID_VIDEO_RES_T


  UINT16 ui2_sink_colorimetry;//use EDID_VIDEO_COLORIMETRY_T

  UINT8 ui1_sink_rgb_color_bit;//color bit for RGB
  UINT8 ui1_sink_ycbcr_color_bit; // color bit for YCbCr

  UINT16 ui2_sink_aud_dec;// use EDID_AUDIO_DECODER_T
  UINT8 ui1_sink_is_plug_in;//1: Plug in 0:Plug Out
  UINT32 ui4_hdmi_dst_ch_type;//use EDID_A_FMT_CH_TYPE
  UINT32 ui4_hdmi_dst_ch3ch4ch5ch7_type;//use EDID_A_FMT_CH_TYPE1
  UINT32 ui4_hdmi_dsd_ch_type;//use EDID_A_FMT_CH_TYPE
  UINT32 ui4_hdmi_dsd_ch3ch4ch5ch7_type;//use EDID_A_FMT_CH_TYPE1
  UINT32 ui4_hdmi_pcm_ch_type;//use EDID_A_FMT_CH_TYPE
  UINT32 ui4_hdmi_pcm_ch3ch4ch5ch7_type;//use EDID_A_FMT_CH_TYPE1

  UINT32 ui4_dac_dst_ch_type;//use EDID_A_FMT_CH_TYPE
  UINT32 ui4_dac_dsd_ch_type;//use EDID_A_FMT_CH_TYPE
  UINT32 ui4_dac_pcm_ch_type;//use EDID_A_FMT_CH_TYPE

  UINT8 ui1_sink_i_latency_present;
  UINT8 ui1_sink_p_audio_latency;
  UINT8 ui1_sink_p_video_latency;
  UINT8 ui1_sink_i_audio_latency;
  UINT8 ui1_sink_i_video_latency;

  UINT8 ui1HdmiOutputMode;
  UINT8 ui1ExtEdid_Revision;
  UINT8 ui1Edid_Version;
  UINT8 ui1Edid_Revision;
  UINT8 ui1_Display_Horizontal_Size;
  UINT8 ui1_Display_Vertical_Size;    
  UINT32 ui4_ID_Serial_Number;
  #if 1//CONFIG_DRV_3D_SUPPORT//
  UINT32 ui4_sink_cea_3D_resolution;
  #endif
  UINT8 ui1_sink_support_ai;//0: not support AI, 1:support AI
  UINT16 ui2_sink_cec_address;  
}   HDMI_EDID_INFO_T;




/*-----------------------------------------------------------------------------
                    Notify function (AVD to SCOM)
 ----------------------------------------------------------------------------*/
 /* Notify conditions */
typedef enum
{
    AVD_DETECT_HOT_PLUG_IN = -1,
    AVD_DETECT_HOT_PLUG_OUT,
    AVD_EDID_READY_OK,
    AVD_CHG_SACD_OUTPUT_TYPE,
    AVD_NOTIFY_HDCP_FAIL,
    AVD_NOTIFY_HDCP_OK,
    AVD_NOTIFY_CHG_VIDEO_RES_OK
}   AVD_COND_T;

/* Notify function */

#if (UNIFORM_DRV_CALLBACK)//for New Linux driver
typedef   struct  _AVD_CB_INFO_T
{
 AVD_COND_T e_nfy_cond;
 VOID*        pv_nfy_info;
}  AVD_CB_INFO_T;

#else

typedef VOID (*x_avd_nfy_fct) (
  AVD_COND_T e_nfy_cond,
  VOID*        pv_nfy_info);

#endif


typedef   struct  _SRM_INFO_T
{
  UINT32 u4ByteCount;
  UINT8 __cross_space__ *prSrmDataAddr;

}  SRM_INFO_T;


typedef struct _SO_NY_AP_TO_AVD_REENCODE_PARAMETER_T
{
  BOOL fgDspReEncodeOn;//if AP enable DSP ReEncode, then it will be TRUE
  BOOL fgLosslessByPassMixerflag;//audio bistream content lossless_may_bypass_mixer flag
  BOOL fgSAorIA;//Bitstream Content Second audio(SA) or Interactive audio(IA) is ON, then it will be TRUE
  BOOL fgPrimaryAudioExist;//There is primary audio exist: TRUE, otherwise:FALSE

  UINT8  u1Disc_Type;//see DISC_TYPE
  UINT8  u1File_Type;//see AVD_FILE_TYPE

} SO_NY_AP_TO_AVD_REENCODE_PARAMETER_T;

typedef   struct  _RE_ENCODE_INFO_T
{
BOOL bHdmiReencode;
BOOL bSpdiffReencode;
UINT32 u4Disc_Type;
}  RE_ENCODE_INFO_T;

typedef struct _AVD_INFO_T
{
    BOOL fgBDPType;
    BOOL fgReEncode;
    BOOL fgByPassMixerflag;
    BOOL fgSAorIA;
    UINT32 u4Disc_Type;
    BOOL fgHDMIExteDecode;
    BOOL fgIEC48Khz;
    UINT32 u4DigitalOutType;
} AVD_INFO_T;

typedef struct _PHILI_AVD_INFO_T
{
    BOOL fgBDPType;
    BOOL fgReEncode;
    BOOL fgByPassMixerflag;
    BOOL fgSAorIA;
    UINT32 u4Disc_Type;
    BOOL fgHDMIExteDecode;
    BOOL fgIEC48Khz;
    UINT32 u4DigitalOutType;
} PHILI_AVD_INFO_T;

typedef struct _SO_NY_AVD_INFO_T
{
  AVD_AUD_OUT_PRIORITY _u1UI_Aud_Priority;//AVD_AUD_OUT_PRIORITY
  AVD_BD_AUD_SETTING  _u1UI_BD_Aud_Setting;//AVD_BD_AUD_SETTING
  AVD_COAXIAL_DOLBY_DIGITAL_SETTING _u1UI_Dolby_Digital_Setting;//AVD_COAXIAL_DOLBY_DIGITAL_SETTING
  AVD_COAXIAL_DTS_SETTING _u1UI_Dts_Setting;//AVD_COAXIAL_DTS_SETTING
  AVD_DSD_SETTING _u1UI_DSD_Setting;
  SO_NY_AP_TO_AVD_REENCODE_PARAMETER_T t_Reencode_Param;
  BD_DVD_AUDIO_OUTPUT_SETTING _u1UI_BD_DVD_Audio_output_Setting;
  AVD_AAC_SETTING _u1UI_AAC_Setting;

} SO_NY_AVD_INFO_T;


typedef enum _ACP_TYPE_T {
    ACP_GENRAL_AUDIO=0,
    ACP_IEC958_AUDIO,
    ACP_DVD_AUDIO,
    ACP_SACD
} ACP_TYPE_T;

typedef struct _HDMI_ACP_INFO_T {
    ACP_TYPE_T eType;
    UINT8 au1AcpData[16];
} HDMI_ACP_INFO_T;

typedef struct _HDMI_SPD_INFO_T {
    UINT8 au1SpdData[25];
} HDMI_SPD_INFO_T;



#endif


