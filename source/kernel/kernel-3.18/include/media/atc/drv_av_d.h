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

#ifndef _DRV_AV_D_H_
#define _DRV_AV_D_H_

#include "x_typedef.h"
#include "x_os.h"
#include "x_avd.h"
#include "chip_ver.h"
#include "x_aud_dec.h"

//#include "drv_if_pmx.h"
// #include "x_drv_cli.h"
//#include "drv_aud.h"

#define PMX_HW_PLANE_1    0 // vdo front
#define PMX_HW_PLANE_2    1 // vdo rear
#define PMX_HW_PLANE_3    2 // osd 1
#define PMX_HW_PLANE_4    3 // osd 2
#define PMX_HW_PLANE_5    4 // osd 3
#define PMX_HW_PLANE_6    5 // osd 4
//#define PMX_HW_PLANE_7    6 // osd 5
#define PMX_HW_PLANE_8    7 // osd_r_2
#define PMX_HW_PLANE_9    8 // osd_r_3
//#define PMX_MIX_DVD2AP    9 // dvd vdo to ap panel
#define PMX_MIX_AP2DVD   10 // ap osd_r mix dvd video to tve

#define PMX_1             0 // PMX Front
#define PMX_2             1 // PMX Rear

// Plane mixer TV type
#define PMX_TV_TYPE_NTSC        0
#define PMX_TV_TYPE_PAL_M       1
#define PMX_TV_TYPE_PAL_N       2
#define PMX_TV_TYPE_PAL         3

// *********************************************************************
// Enum and struct definitions
// *********************************************************************
//For HDCP on/off
#define PMX_HDCP_ON  0
#define PMX_HDCP_OFF 1
#define PMX_HDCP_LOGO_ON 2//only for SLT Test 

#define OSD_CFG_IS_REAL_INTERLACE  0//added by +e
//#define HBR_USE_I2S
// *********************************************************************
// Enum For Video driver PMX an VDP relative
// *********************************************************************
//PMX set command condition
typedef enum
{
  PMX_CHG_HDMI_COLOR_SPACE,
  PMX_CHG_HDMI_VIDEO_ASPECT_RATIO, 
  PMX_SET_UI_TV_SYSTEM,
  PMX_CHG_VIDEO_BISTREAM_TYPE, 
  PMX_SET_HDMI_BRIGHTNESS,
  PMX_SET_HDMI_CONTRAST,
  PMX_SET_HDMI_HUE,
  PMX_SET_HDMI_SATURATION,
  PMX_CHG_DEEP_COLOR_BIT,
  PMX_SET_VIDEO_PLL,
  PMX_SET_HDMI_RES_CHG,
  PMX_SET_METADATA,
  PMX_HDCP_ON_OFF,
  PMX_CHG_UI_RES_AUTO,
  PMX_TURN_ON_OFF_HDMI,
  PMX_SET_HDMI_SHARPNESS,
  PMX_SET_CCIR_RES_CHG,
  PMX_SET_EXT_SHARPNESS,
  PMX_SET_EXT_BRIGHTNESS,
  PMX_SET_EXT_CONTRAST,
  PMX_SET_EXT_SATURATION,
  PMX_SET_EXT_COLOR_SPACE,
  PMX_SET_EXT_Y_C_DELAY,
  PMX_SET_EXT_CUE_CORRECT,
  PMX_SET_EXT_BORDER_LEVEL,
  PMX_SET_EXT_DEINTERLACE_MODE,
  PMX_SET_EXT_NOISE_REDUCTION,
  PMX_SET_EXT_EDGE_ENHANCEMENT,
  PMX_SET_EXT_DETAIL_ENHANCEMENT,
  PMX_SET_EXT_ZOOM_IMAGE,
  PMX_SET_EXT_GAMMA,
  PMX_SET_EXT_DEEP_COLOR      
}   PMX_SET_AVD_COND_T;

//VDP set command condition
typedef enum
{
 // VDP_SET_VIDEO_PLL = 0, //temply
 // VDP_SEND_HDMI_TVE_RES_CHG,//temply
  VDP_SET_BITSTREAM_CHG=0  
  
} VDP_SET_AVD_COND_T; 

//HDMI RX set command condition
typedef enum
{
 
  HDMI_RX_SET_VIDEO_CHG_MODE = 0,
    
  
} HDMI_RX_SET_AVD_COND_T; 


//PMX get command condition
typedef enum
{
  PMX_GET_SINK_RES_SUPPORT = 0,
  PMX_GET_VALID_RES_AND_VFREQ,
  PMX_GET_HDMI_MAX_COLOR_BIT,
  PMX_GET_HDMI_COLOR_SPACE_SUPPORT
} PMX_DRV_GET_AVD_TYPE_T;


typedef  enum
{
   HDMI_4_3 = 0,
   HDMI_16_9,

} HDMI_VIDEO_ASPECT_T;


typedef  enum
{
   HDMI_RGB = 0,
   HDMI_RGB_FULL,
   HDMI_YCBCR_444,
   HDMI_YCBCR_422,
   HDMI_XV_YCC, 
   HDMI_YCBCR_444_FULL,
   HDMI_YCBCR_422_FULL

} PMX_HDMI_OUT_COLOR_SPACE_T;

typedef  enum
{
   GAMMA_STANDARD = 0, 
   GAMMA_BRIGHTER , 
   GAMMA_THEATRE,
   
} GAMMA_MODE_SETTING;

typedef PMX_HDMI_OUT_COLOR_SPACE_T HDMI_COLOR_SPACE_T ;

typedef enum
{
   NTSC_SYS=0,
   PAL_SYS,
   AUTO_DEFAULT_NTSC_SYS,
   AUTO_DEFAULT_PAL_SYS,  
} AVD_TV_SYSTEM_TYPE_T;//HDMI_TV_SYSTEM_TYPE_T;

typedef enum
{
 PAL_VIDEO_TYPE=0,
 NTSC_VIDEO_TYPE 
} VIDEO_BITSTEAM_TYPE_T;


typedef enum
{
  HDMI_DEEP_COLOR_AUTO=0,	
  HDMI_NO_DEEP_COLOR,
  HDMI_DEEP_COLOR_10_BIT,
  HDMI_DEEP_COLOR_12_BIT,
  HDMI_DEEP_COLOR_16_BIT 
} HDMI_DEEP_COLOR_T;

typedef enum
{
  NOAH_OUTPUT_DEPTH_INVALID=0,	
  NOAH_OUTPUT_DEPTH_422_8BIT,
  NOAH_OUTPUT_DEPTH_422_10BIT,
  NOAH_OUTPUT_DEPTH_422_12BIT,
  NOAH_OUTPUT_DEPTH_444_8BIT, 
  NOAH_OUTPUT_DEPTH_444_10BIT,
  NOAH_OUTPUT_DEPTH_444_12BIT
  
} NOAH_OUTPUT_DEPTH_T;

typedef  enum
{
  SCALE_DOWN=0,
  SCALE_UP

} VIDEO_SCALE_MODE_T;

//#ifndef NEW_PMX_RESOLUTION_MODE

typedef  enum
{
    RES_480I=0, 
    RES_576I,
    RES_480P,
    RES_576P,
    RES_480P_800,  // 4
    RES_600P_800,  // 5
    RES_600P_1024, // 6
    RES_720P_1280, // 7
    RES_800P_1280, // 8
    RES_768P_1024, // 9
    RES_576P_1440, // 10
    RES_480P_2880,//6
    RES_576P_2880,//7    
    RES_720P60HZ,//8
    RES_720P50HZ,//9
    RES_1080I60HZ,//10
    RES_1080I50HZ,//11
    RES_1080P60HZ,//12
    RES_1080P50HZ,//13
    RES_1080P30HZ,//14
    RES_1080P25HZ, //15
    RES_480I_2880,//16
    RES_576I_2880,//17
    RES_1080P24HZ, //18
    RES_1080P23_976HZ, //19, 1080P23.976hz
    RES_1080P29_97HZ, //20, 1080P29.97hz
    RES_3D_1080P23HZ, //21, 1080p47.952Hz
    RES_3D_1080P24HZ, //22, 1080p48hz
    RES_3D_720P60HZ, //23, 720p120hz
    RES_3D_720P50HZ, //24, 720p100hz
    RES_3D_720P30HZ, //25, 720p120hz
    RES_3D_720P25HZ, //26, 720p100hz
    RES_3D_576P50HZ, //27, 576p100hz
    RES_3D_480P60HZ, //28, 480p120hz
    RES_3D_1080I60HZ, //29, 1080i120hz
    RES_3D_1080I50HZ, //30, 1080i100hz
    RES_3D_1080I30HZ, //31, 1080i120hz
    RES_3D_1080I25HZ, //32, 1080i100hz
    RES_3D_576I25HZ, //33, 576i100hz
    RES_3D_480I30HZ, //34, 480i120hz
    RES_3D_576I50HZ, //35, 576i100hz
    RES_3D_480I60HZ, //36, 480i120hz
    RES_2D_480I60HZ, //37, 480i60hz
    RES_2D_576I50HZ, //38, 576i50hz
    RES_PANEL_AUO_B089AW01, //Total: 1344x625, Act: 1024x600, Frm: 60Hz, Clk: 50.4MHz
    RES_2160P,
    RES_480P_800_50HZ,//4
    RES_600P_800_50HZ,
    RES_600P_1024_50HZ,
    RES_720P_1280_50HZ,
    RES_800P_1280_50HZ,
    RES_MODE_NUM,          // dummy mode, used to determine the last mode    
    RES_AUTO
}   PMX_RESOLUTION_MODE_T;
//#endif

//CONFIG_DRV_3D_SUPPORT 
#define fgIs3DOutput(u1Res) ((u1Res == RES_3D_1080P23HZ) || (u1Res == RES_3D_1080P24HZ) ||\
	(u1Res == RES_3D_720P60HZ) || (u1Res == RES_3D_720P50HZ) || (u1Res == RES_3D_720P30HZ) ||\
	(u1Res == RES_3D_720P25HZ) || (u1Res == RES_3D_576P50HZ) || (u1Res == RES_3D_480P60HZ) ||\
	(u1Res == RES_3D_1080I60HZ) || (u1Res == RES_3D_1080I50HZ) || (u1Res == RES_3D_1080I30HZ) ||\
	(u1Res == RES_3D_1080I25HZ) || (u1Res == RES_3D_576I25HZ) || (u1Res == RES_3D_480I30HZ) ||\
	(u1Res == RES_3D_576I50HZ) || (u1Res == RES_3D_480I60HZ))

#define fgIsHDRes(u1Res) ((u1Res == RES_720P60HZ)||(u1Res == RES_720P50HZ)||(u1Res == RES_1080I60HZ)||\
                                           (u1Res == RES_1080I50HZ)||(u1Res == RES_1080P60HZ)||(u1Res == RES_1080P50HZ)||\
                                           (u1Res == RES_1080P30HZ)||(u1Res == RES_1080P25HZ)||(u1Res == RES_1080P24HZ)||\
                                           (u1Res == RES_1080P23_976HZ)||(u1Res == RES_1080P29_97HZ) ||(u1Res == RES_3D_1080P23HZ) ||\
                                           (u1Res == RES_3D_1080P24HZ) ||(u1Res == RES_3D_720P60HZ) ||(u1Res == RES_3D_720P50HZ) ||\
                                           (u1Res == RES_3D_720P30HZ) ||(u1Res == RES_3D_720P25HZ) ||(u1Res == RES_3D_1080I60HZ) ||\
                                           (u1Res == RES_3D_1080I50HZ) ||(u1Res == RES_3D_1080I30HZ) ||(u1Res == RES_3D_1080I25HZ))

#define fgIsTrueInterlaceSupport(u1Res) ((u1Res == RES_2D_480I60HZ)||(u1Res == RES_2D_576I50HZ)||(u1Res == RES_3D_1080I60HZ) || (u1Res == RES_3D_1080I50HZ) ||\
	                                                              (u1Res == RES_3D_1080I30HZ) ||(u1Res == RES_3D_1080I25HZ) ||(u1Res == RES_3D_576I25HZ) || (u1Res == RES_3D_480I30HZ) ||\
	                                                              (u1Res == RES_3D_576I50HZ) || (u1Res == RES_3D_480I60HZ))
	                                                              
#define fgNeedNewSDMode(u1Res) ((u1Res == RES_480P)||(u1Res == RES_576P))

#define fgIsPanelRes(u1Res) (u1Res == RES_PANEL_AUO_B089AW01)
	                                                              
typedef  enum
{
    VSYNC_3D_P_L=0,
    VSYNC_3D_P_R,
    VSYNC_3D_I_L_T,
    VSYNC_3D_I_R_T,
    VSYNC_3D_I_L_B,
    VSYNC_3D_I_R_B,
    VSYNC_3D_UNKNOWN,
    VSYNC_2D_P,
    VSYNC_2D_I_T,
    VSYNC_2D_I_B,
    VSYNC_2D_UNKNOWN,
} PMX_VSYNC_MODE_T;

typedef enum
{
  PMX_3D_LR,
  PMX_3D_LL  	
} PMX_3D_MODE_T;

//#ifdef NEW_PMX_RESOLUTION_MODE
typedef PMX_RESOLUTION_MODE_T HDMI_RESOLUTION_T ;

//#endif
#if 0
typedef  enum
{
    AUTO_DEFAULT_480I=0,
    AUTO_DEFAULT_576I,
    AUTO_DEFAULT_480P, 
    AUTO_DEFAULT_576P, 
    AUTO_DEFAULT_480P_1440,
    AUTO_DEFAULT_576P_1440,
    AUTO_DEFAULT_480P_2880,
    AUTO_DEFAULT_576P_2880,
    AUTO_DEFAULT_720P60HZ,
    AUTO_DEFAULT_720P50HZ,
    AUTO_DEFAULT_1080I60HZ,
    AUTO_DEFAULT_1080I50HZ,
    AUTO_DEFAULT_1080P60HZ,
    AUTO_DEFAULT_1080P50HZ,
    AUTO_DEFAULT_1080P30HZ,
    AUTO_DEFAULT_1080P25HZ  
}HDMI_AUTO_RESOLUTION_T;
#endif

    

typedef  enum
{
  YLPF_SHARP_NORMAL=0,	
  YLPF_SHARP_MIDLLE,
  YLPF_SHARP_STRONG ,
  YLPF_OFF
} HDMI_YLPF_MODE_T;


typedef  enum
{
  GAMUT_TYPE_AVCHD = 0,
  GAMUT_TYPE_AVCHD_FIXED = 1,
  

} XVYCC_GAMUT_TYPE_T;

// *********************************************************************
// Enum For Audio driver relative
// *********************************************************************

typedef  enum
{
   HDMI_AUDIO_AUTO = 0,
   HDMI_AUDIO_PCM_2CH,
   HDMI_AUDIO_PCM_MULTI,
   HDMI_AUDIO_OFF,
   HDMI_AUDIO_REENCODE
} HDMI_AUDIO_OUT_TYPE_T;


typedef enum
{
  HDMI_ACP_GENRAL_AUDIO=0,	
  HDMI_ACP_IEC958_AUDIO,	
  HDMI_ACP_DVD_AUDIO,
  HDMI_ACP_SACD
} HDMI_ACP_TYPE_T;

typedef enum
{
  HDMI_ISRC1_GENRAL_AUDIO=0,	
  HDMI_ISRC1_DVD_AUDIO
  
} HDMI_ISRC1_TYPE_T;

typedef  enum
{
   AUDIO_DEC_DRV_SET_AUDIO_SRC_CHG = 0,
  AUDIO_DEC_DRV_SET_PROHIBIT_DOWN_MIX,
  AUDIO_DEC_DRV_SET_HDMI_CH_STATUS,
 // AUDIO_DEC_DRV_SET_AUDIO_OUTPUT_CHANNEL, 
 // AUDIO_DEC_DRV_SET_AUDIO_INPUT_CHANNEL_NUMBER,   
  AUDIO_DEC_DRV_SET_HDMI_SPK_MAPPING,
  AUDIO_DEC_DRV_SET_HDMI_OUTPUT_TYPE


}   AUDIO_NFY_AVD_COND_T;




typedef enum// new add 2007/9/12
{
  FS_16K= 0x00,
  FS_22K,
  FS_24K,    
  FS_32K,     
  FS_44K,     
  FS_48K,     
  FS_64K,     
  FS_88K,     
  FS_96K,     
  FS_176K,   
  FS_192K,   
  FS512_44K,//for DSD
  FS_768K,
  FS128_44k,
  FS_8K, //for 8K flow.
  FS_UNKNOWN
} AUDIO_SAMPLING_T;


typedef enum
{
  IEC_48K = 0,
  IEC_96K, 
  IEC_192K, 
  IEC_768K,
  IEC_44K, 
  IEC_88K,
  IEC_176K,
  IEC_705K,
  IEC_16K,
  IEC_22K,
  IEC_24K,    
  IEC_32K,   
  IEC_UNKNOWN
} IEC_FRAME_RATE_T;


typedef enum
{
  FMT_384FS = 0,
  FMT_256FS
} XCK_FS_FMT_T;

typedef enum
{
  AVD_BITS_NONE=0,	
  AVD_LPCM=1,
  AVD_AC3,
  AVD_MPEG1_AUD, 
  AVD_MP3,      
  AVD_MPEG2_AUD,       
  AVD_AAC,           
  AVD_DTS,         
  AVD_ATRAC,       
  AVD_DSD,         
  AVD_DOLBY_PLUS,  
  AVD_DTS_HD,      
  AVD_MAT_MLP,     
  AVD_DST,         
  AVD_WMA,
  AVD_CDDA,
  AVD_SACD_PCM,
  AVD_HDCD =0xfe,
  AVD_BITS_OTHERS=0xff
} AUDIO_BITSTREAM_TYPE_T;


// *********************************************************************
// Enum For HDMI driver relative
// *********************************************************************
//
typedef enum
{
  HDMI_FS_32K = 0,  	
  HDMI_FS_44K,  
  HDMI_FS_48K,    
  HDMI_FS_88K,  
  HDMI_FS_96K, 
  HDMI_FS_176K,  
  HDMI_FS_192K             

} HDMI_AUDIO_SAMPLING_T;

typedef enum
{
  HDMI_SET_VIDEO_RES_CHG = 1,
  HDMI_SET_AUDIO_OUT_CHANNEL,
  HDMI_SET_AUDIO_OUTPUT_TYPE,
  HDMI_SET_AUDIO_PACKET_OFF,
  HDMI_SET_VIDEO_COLOR_SPACE,
  HDMI_SET_VIDEO_CONTRAST,
  HDMI_SET_VIDEO_BRIGHTNESS,
  HDMI_SET_VIDEO_HUE,
  HDMI_SET_VIDEO_SATURATION,
  HDMI_SET_ASPECT_RATIO,
  HDMI_SET_AVD_NFY_FCT,
  HDMI_SET_TURN_OFF_TMDS,
  HDMI_SET_AUDIO_CHG_SETTING,
  HDMI_SET_AVD_INF_ADDRESS,
  HDMI_SET_HDCP_INITIAL_AUTH,
  HDMI_SET_VPLL,
  HDMI_SET_SOFT_NCTS,
  HDMI_SET_HDCP_OFF,
  HDMI_SET_VIDEO_SHARPNESS,
  HDMI_SET_EXT_SHARPNESS,
  HDMI_SET_EXT_BRIGHTNESS,
  HDMI_SET_EXT_CONTRAST,
  HDMI_SET_EXT_SATURATION,
  HDMI_SET_EXT_COLOR_SPACE,
  HDMI_SET_EXT_Y_C_DELAY,
  HDMI_SET_EXT_CUE_CORRECT,
  HDMI_SET_EXT_BORDER_LEVEL,
  HDMI_SET_EXT_DEINTERLACE_MODE,
  HDMI_SET_EXT_NOISE_REDUCTION,
  HDMI_SET_EXT_EDGE_ENHANCEMENT,
  HDMI_SET_EXT_DETAIL_ENHANCEMENT,
  HDMI_SET_EXT_ZOOM_IMAGE,
  HDMI_SET_EXT_GAMMA
  
}   AV_D_HDMI_DRV_SET_TYPE_T;

typedef enum
{
  DRV_HDMI_NFY_SINK_PLUG=0,
  DRV_HDMI_NFY_EDID_READ ,
  DRV_HDMI_NFY_CHG_RES,
  DRV_HDMI_NFY_CHG_NCTS,
  DRV_HDMI_NFY_CHG_SCALE,
  DRV_HDMI_NFY_COMMAND_DONE,
  DRV_HDMI_NFY_POWER_ON_EDID_OK,  
  DRV_HDMI_NFY_HDCP_OK,
  DRV_HDMI_NFY_HDCP_FAIL,  
  DRV_HDMI_NFY_POWER_ON_HOT_PLUG_OUT,
  
} HDMI_AVD_COND_T;

typedef enum
{
  HDMI_PLUG_IN_ONLY = 0,
  HDMI_PLUG_IN_AND_SINK_POWER_ON,
  HDMI_PLUG_OUT
}  HDMI_NFY_PLUG_STATE_T;


typedef enum
{
  HDMI_EDID_NOT_READY = 0,
  HDMI_EDID_IS_READY,
  HDMI_EDID_IS_ERROR
}  HDMI_NFY_EDID_STATE_T;

typedef enum
{
  HDMI_IS_CHG_RES_START = 0,
  HDMI_CHG_RES_END
} HDMI_NFY_CHG_RES_STATE_T;

typedef enum
{
  HDMI_IS_CHG_NCTS_START = 0,
  HDMI_CHG_NCTS_READY
}  HDMI_NFY_CHG_NCTS_STATE_T;



typedef enum
{
  HDMI_GET_TYPE_SINK_AV_CAP = 0,
  HDMI_GET_TYPE_HOTPLUG_STATE,
  HDMI_GET_TYPE_READ_EDID_STATE,
  HDMI_GET_TYPE_CEC_ADDRESS
  
  

}  AV_D_HDMI_DRV_GET_TYPE_T;

typedef enum
{
  HDMI_SINK_NO_DEEP_COLOR=0,
  HDMI_SINK_DEEP_COLOR_10_BIT=(1<<0),
  HDMI_SINK_DEEP_COLOR_12_BIT=(1<<1),
  HDMI_SINK_DEEP_COLOR_16_BIT=(1<<2) 
} HDMI_SINK_DEEP_COLOR_T;


typedef  enum
{
  SV_I2S = 0,
  SV_SPDIF   
 
} HDMI_AUDIO_INPUT_TYPE_T;  


typedef  enum
{
  HDMI_RJT_24BIT= 0,
  HDMI_RJT_16BIT,
  HDMI_LJT_24BIT, 
  HDMI_LJT_16BIT,
  HDMI_I2S_24BIT,
  HDMI_I2S_16BIT 
} HDMI_AUDIO_I2S_FMT_T;  

typedef enum
{
  MAP_I2S_DATA0=0,
  MAP_I2S_DATA1,
  MAP_I2S_DATA2,
  MAP_I2S_DATA3,
  MAP_I2S_DATA4

} I2S_DATA_MAP_T;


typedef enum
{
  PCM_16BIT=0,
  PCM_20BIT,
  PCM_24BIT

} PCM_BIT_SIZE_T;






//for hdmi_if.c
typedef enum
{
 CHG_NCTS_AND_INPUT =0,
 CHG_NCTS_ONLY ,
 CHG_INPUT_ONLY
} HDMI_AUDIO_CHG_FUNCTION_TYPE_T;
  
typedef enum
{
  HDMI_INTERNAL_MODE = 0,  
  HDMI_EXTERNAL_TX,  
  HDMI_EXTERNAL_SCL  	
  

} HDMI_OUTPUT_MODE_T;  
  
typedef enum
{
  CCIR_INVALID = 0,
  CCIR_601_MODE ,  	
  CCIR_656_MODE 
} CCIR_IN_OUT_MODE_T; 

typedef enum
{
  CCIR_NON_USED = 0,
  CCIR_YCMIX_8BIT,  	
  CCIR_YCMIX_10BIT,
  CCIR_YCMIX_12BIT,  
  CCIR_YC422_16BIT,
  CCIR_YC422_20BIT,
  CCIR_YC422_24BIT,
  CCIR_YC444_24BIT,
  CCIR_YC444_30BIT,
  CCIR_YC444_36BIT,
} CCIR_IN_OUT_FORMAT_T; 

typedef enum
{
  CCIR_SRC_FROM_FMT_WO_MSG = 0,
  CCIR_SRC_FROM_FMT_W_MSG,
  CCIR_SRC_FROM_SCALER_WO_MSG,
  CCIR_SRC_FROM_SCALER_W_MSG,

} CCIR_SRC_TYPE_E;

typedef enum
{
  HDMI_AUD_SRC_2N = 1,  	
  HDMI_AUD_SRC_4N = 2,  	  	
  

} HDMI_AUD_SRC_MODE_T;  

typedef enum
{
  HDMI_DEBUG_EDID = (1 << 0),  	
  HDMI_DEBUG_HOT_PLUG = (1 << 1),  	
  HDMI_DEBUG_HDCP = (1 << 2),  	
  HDMI_DEBUG_HDCP_RI = (1 << 3),
  HDMI_DEBUG_SRM = (1 << 4),  	
  HDMI_DEBUG_HDMI_AUDIO = (1 << 5),  
  HDMI_DEBUG_DUMP_EDID = (1 << 6),  
  HDMI_DEBUG_XVYCC = (1 << 7), 
  HDMI_DEBUG_DEEP_COLOR = (1 << 8),
  HDMI_DEBUG_PICTURE_MODE = (1 << 9),
  HDMI_DEBUG_3D = (1 << 10),
  HDMI_DEBUG_ALL = (1 << 23),//max 23
} HDMI_DEBUG_MESSAGE_T;
 
// *********************************************************************
// Struct For Video driver PMX an VDP relative
// *********************************************************************


typedef struct  _PMX_SET_TV_SYS_PARAMETER_T
{
  UINT16 u2_Vfreq; //60,50,24,25,30 HZ,  default V freq
  BOOL  fgAutoEnable; //TRUE enable auto mode
   
} PMX_SET_TV_SYS_PARAMETER_T;

typedef struct  _CCIR_DRV_INFO_T
{
  CCIR_IN_OUT_MODE_T eDgoMode;
  CCIR_IN_OUT_FORMAT_T eDgoFmt;
  CCIR_IN_OUT_MODE_T eDgiMode;
  CCIR_IN_OUT_FORMAT_T eDgiFmt;  
   
} CCIR_DRV_INFO_T;

typedef struct  _CCIR_IN_DRV_INFO_T
{
  CCIR_IN_OUT_MODE_T eDgiMode;
  CCIR_IN_OUT_FORMAT_T eDgiFmt;  
   
} CCIR_IN_DRV_INFO_T;

typedef struct _PMX_SET_HDMI_RES_PARAMETER_T
{
   UINT8  u1ScaleMode;// VIDEO_SCALE_MODE_T
   UCHAR ucHdmiOutDisplayResolution; 
   UCHAR ucFmtOutResolution; 
   UCHAR ucCCIROutResolution; 
   UCHAR ucHdmiOutMode; 
   CCIR_DRV_INFO_T eCcirDrvInfo;
   BOOL fgOsdMsg;
   UCHAR ucCCIROutMode; //
   BOOL fgCCIROutEnable; //
   BOOL fgHdmiOutEnable; //
   UINT16 u2_Vfreq; //60,50,24,25,30 HZ
   
   
} PMX_SET_HDMI_RES_PARAMETER_T;



typedef struct  _PMX_SET_HDMI_COLOR_SPACE_PARAMETER_T
{
   UINT8  u1UIColorSpace; // The UI Color space, PMX_HDMI_COLOR_SPACE_T
   BOOL *fgValid;   //AV-D return TRUE if u1UIColorSpace is supported
   BOOL *fgReqChgRes; // TRUE: AV-D request PMX change HDMI resolution

   
} PMX_SET_HDMI_COLOR_SPACE_PARAMETER_T;

typedef struct  _PMX_SET_HDMI_COLOR_BIT_PARAMETER_T
{
  UINT8  u1UIColorBit; // The UI Color space, PMX_HDMI_DEEP_COLOR_BIT_TYPE_T
  BOOL *fgValid;   //AV-D return TRUE if u1UIColorBit is supported
  BOOL *fgReqChgRes; // TRUE: AV-D request PMX change resolution

   
} PMX_SET_HDMI_COLOR_BIT_PARAMETER_T;



typedef struct _PMX_GET_AVD_VALID_RES_AND_VFREQ_PARAMETER_T
{
  UINT32 u4ResCheck;//The resolution which PMX  want AV-D to check  //define in drv_if_pmx.h?
  //UINT8  u1UITVSystem; // The UI TV system set for UI(  AVD_TV_SYSTEM_TYPE_T)
  UINT16 u2VsyncFreq;  
  UINT32 u4ValidAutoRes; //AV-D return a resolution for Auto
  UINT16 u2ValidAutoVfreq; //AV-D return valid Vfreq, 60,50,24,25,30 HZ according to TV system setting
  //UINT32 *fgResValid;   //AV-D return TRUE if u4ResCheck is supported
} PMX_GET_AVD_VALID_RES_AND_VFREQ_PARAMETER_T;

typedef struct _PMX_GET_AVD_RES_SUPPORT_PARAMETER_T
{
    UINT32  *u4AvdRetunResSupport; // AV-D return TV Detail information for supported resolution 
                                 // see definition for HDMI_SINK_VIDEO_RES_T

} PMX_GET_AVD_RES_SUPPORT_PARAMETER_T;


// *********************************************************************
// Struct For HDMI driver and audio driver
// *********************************************************************
 
typedef struct _AUDIO_DEC_OUTPUT_CHANNEL_T
{
  UINT16 FL: 1; //bit0
  UINT16 FR: 1; //bit1
  UINT16 LFE: 1; //bit2
  UINT16 FC: 1; //bit3
  UINT16 RL: 1; //bit4
  UINT16 RR: 1; //bit5
  UINT16 RC: 1; //bit6
  UINT16 FLC: 1; //bit7
  UINT16 FRC: 1; //bit8
  UINT16 RRC: 1; //bit9
  UINT16 RLC: 1; //bit10
  
} HDMI_AUDIO_DEC_OUTPUT_CHANNEL_T;

typedef union _AUDIO_DEC_OUTPUT_CHANNEL_UNION_T
{
  HDMI_AUDIO_DEC_OUTPUT_CHANNEL_T bit;//HDMI_AUDIO_DEC_OUTPUT_CHANNEL_T
  UINT16 word;
   
} AUDIO_DEC_OUTPUT_CHANNEL_UNION_T;


//e_spk_map[0]: FL/FR Channel mapping 
//e_spk_map[1]: RL/RR Channel mapping
//e_spk_map[2]:  FC Channel mapping
//e_spk_map[3]:  LFE Channel mapping  
//e_spk_map[4]:  FLC/FRC Channel mapping
//e_spk_map[5]:  RLC/RRC Channel mapping                                                                                
//e_spk_map[6]:  RC Channel mapping  
typedef struct _HDMI_AUDIO_DEC_DRV_SPK_MAP_T
{
  I2S_DATA_MAP_T e_spk_map[7];

} HDMI_AUDIO_DEC_DRV_SPK_MAP_T;


typedef struct _HDMI_AUDIO_SPK_AND_FS_INFO_T
{
  BYTE bMaxChForFs[7]; //Max channel number for each fs
  BYTE bMaxFsForMultiCh;//Max fs for multi channel
  BYTE bMaxFsFor2Ch;//Max fs for 2 channel
} HDMI_AUDIO_SPK_AND_FS_INFO_T;


 
typedef struct _AUDIO_DEC_TO_AVD_PARAMETER_T
{
   UINT8 u1AudDecFmt; //for setting e_aud_code from audio driver,  audio decoder format, see drv_aud.h AUD_DRV_FMT_T
   UINT8 u1SrcChannel;
   AUDIO_SAMPLING_T u1SrcSamplingRate;
   IEC_FRAME_RATE_T u1SrcIecFrameRate;
    
} AUDIO_DEC_TO_AVD_PARAMETER_T;


#if 0
typedef struct _VIDEO_TO_AVD_PARAMETER_T
{
   UINT8  u1ScaleMode;
   UINT32  u4OutDisplayResolution; //HDMI_RESOLUTION_T
   //UINT32 u4SrcVideoResolution;//temply, no need
   
} VIDEO_TO_AVD_PARAMETER_T;
#endif

// *********************************************************************
// Struct For Hdmi driver 
// *********************************************************************
typedef   struct _HDMI_SINK_AV_CAP_T
{
  UINT32 ui4_sink_cea_ntsc_resolution;//use HDMI_SINK_VIDEO_RES_T
  UINT32 ui4_sink_cea_pal_resolution;//use HDMI_SINK_VIDEO_RES_T
  UINT32 ui4_sink_dtd_ntsc_resolution;//use HDMI_SINK_VIDEO_RES_T
  UINT32 ui4_sink_dtd_pal_resolution;//use HDMI_SINK_VIDEO_RES_T
  UINT32 ui4_sink_1st_dtd_ntsc_resolution;//use HDMI_SINK_VIDEO_RES_T
  UINT32 ui4_sink_1st_dtd_pal_resolution;//use HDMI_SINK_VIDEO_RES_T  
  UINT32 ui4_sink_native_ntsc_resolution;//use HDMI_SINK_VIDEO_RES_T
  UINT32 ui4_sink_native_pal_resolution;//use HDMI_SINK_VIDEO_RES_T
  UINT16 ui2_sink_colorimetry;//use HDMI_SINK_VIDEO_COLORIMETRY_T
  UINT16 ui2_sink_vcdb_data; //use HDMI_SINK_VCDB_T
  UINT16 ui2_sink_aud_dec;//HDMI_SINK_AUDIO_DECODER_T
  //UINT8 ui1_sink_pcm_ch_num;
  UINT8 ui1_sink_dsd_ch_num;
  UINT8 ui1_sink_pcm_ch_sampling[7];//n: channel number index, value: each bit means sampling rate for this channel number (SINK_AUDIO_32k..)
  UINT8 ui1_sink_pcm_bit_size[7];////n: channel number index, value: each bit means bit size for this channel number
  UINT8 ui1_sink_dst_ch_sampling[7];//n: channel number index, value: each bit means sampling rate for this channel number (SINK_AUDIO_32k..)
  UINT8 ui1_sink_dsd_ch_sampling[7];//n: channel number index, value: each bit means sampling rate for this channel number (SINK_AUDIO_32k..)
  
  UINT8 ui1_sink_spk_allocation;
  UINT8 ui1_sink_content_cnc;
  UINT8 ui1_sink_i_latency_present;
  UINT8 ui1_sink_p_audio_latency;
  UINT8 ui1_sink_p_video_latency;
  UINT8 ui1_sink_i_audio_latency;
  UINT8 ui1_sink_i_video_latency;
  UINT8 e_sink_rgb_color_bit;
  UINT8 e_sink_ycbcr_color_bit; 
  UINT16 ui2_edid_chksum_and_audio_sup;//HDMI_EDID_CHKSUM_AND_AUDIO_SUP_T
  UINT16 ui2_sink_cec_address;
  BOOL   b_sink_edid_ready;
  BOOL   b_sink_support_hdmi_mode;
  UINT8 ui1_ExtEdid_Revision;
  UINT8 ui1_Edid_Version;
  UINT8 ui1_Edid_Revision;
  UINT8 ui1_sink_support_ai;
  UINT8 ui1_Display_Horizontal_Size;
  UINT8 ui1_Display_Vertical_Size;  
   #if 1//CONFIG_DRV_3D_SUPPORT
  BOOL   b_sink_hdmi_video_present;
  BOOL   b_sink_3D_present;
  UINT32 ui4_sink_cea_3D_resolution;
  #endif
}   HDMI_SINK_AV_CAP_T;




//HDMI_AV_INFO_T is information from AVD to HDMI
typedef   struct _HDMI_AV_INFO_T
{
    HDMI_RESOLUTION_T e_resolution;//Final real operating resolution
    HDMI_RESOLUTION_T e_ccir_resolution;
    HDMI_RESOLUTION_T e_fmt_resolution;//formatter
    HDMI_RESOLUTION_T e_memu_resolution;
    UINT8 u1HdmiOutMode;//HDMI_OUTPUT_MODE_T
    CCIR_DRV_INFO_T eCcirDrvInfo;//CCIR_DRV_INFO_T        
    BOOL  fgHdmiOutEnable;
    UINT16 u2VerFreq;//vertical freqency 24,25,30,50,60HZ
    AVD_TV_SYSTEM_TYPE_T e_tv_system;//Final real operating Tv system
    AVD_TV_SYSTEM_TYPE_T e_menu_tv_system;//Menu Tv system
    AVD_TV_SYSTEM_TYPE_T e_disc_tv_system;
    UINT32 u4SrcVideoRes;//temply
    UINT8 u1ScaleMode;
    BYTE  b_menu_auto_resolution_on;
    //BYTE  b_menu_auto_tv_system_on;
    BYTE  b_hotplug_state;
    
    HDMI_VIDEO_ASPECT_T e_menu_video_aspect_ratio;
    UINT8 u1_menu_video_color_space;
    //HDMI_COLOR_SPACE_T e_video_color_space;//Final real output color space
    UINT8 e_video_color_space;//Final real output color space
    HDMI_DEEP_COLOR_T e_menu_deep_color_bit;
    HDMI_DEEP_COLOR_T e_deep_color_bit;
    HDMI_AUDIO_OUT_TYPE_T e_menu_audio_output_type;
    HDMI_AUDIO_SPK_AND_FS_INFO_T st_hdmiAudSpkFsInfo;//for setting dsp
    AUDIO_DEC_OUTPUT_CHANNEL_UNION_T ui2_aud_src_ch;// for audio source input channel mapping
    AUDIO_DEC_OUTPUT_CHANNEL_UNION_T ui2_aud_out_ch;// for audio output channel mapping
    UINT8 ui1_aud_out_ch_number;// for setting dsp output channel number
    UINT8 ui1_audio_input_channel_number;//from audio driver audio channel mapping,
    HDMI_AUDIO_INPUT_TYPE_T e_hdmi_aud_in;    
    BYTE bProhibit;
    AUDIO_SAMPLING_T e_dsp_fs;//from audio driver see x_aud_dec.h AUD_DEC_SAMPLE_FREQ_T
    HDMI_AUDIO_SAMPLING_T e_hdmi_fs;
    AUDIO_BITSTREAM_TYPE_T e_aud_code;//AVD, HDMI internal used    
    HDMI_AUDIO_I2S_FMT_T e_I2sFmt;//I2s format
    HDMI_AUDIO_DEC_DRV_SPK_MAP_T e_i2s_ch_map;
    BYTE bhdmiRChstatus[6];//channel status bit
    BYTE bhdmiLChstatus[6];//channel status bit
    BYTE bAudioRemappingOn;
    
    
    BYTE   bSRCOn;
    BYTE   bSRCRatio;
    BYTE bMuteHdmiAudio;
    IEC_FRAME_RATE_T e_iec_frame;
    XCK_FS_FMT_T e_xck_fs_fmt;//audio xck fs, 384Fs or 256fs
    HDMI_YLPF_MODE_T e_ylpf_mode;
    
   // UINT8 ui1_audio_dec_fmt; //for setting e_aud_code from audio driver,  audio decoder format, see drv_aud.h AUD_DRV_FMT_T
   
   // UINT8 ui1_audio_dec_type;//how many audio channel from audio driver, see x_aud_dec.h AUD_DEC_AUD_TYPE_T
    
    
    INT16 i2_contrast_level;
    INT16 i2_brightness_level;
    INT16 i2_hue_level;
    INT16 i2_saturation_level;
    UINT8 u1GammaMode;     
    UCHAR ucExtSharpnessMode;
    UCHAR ucExtBrightnessLevel;
    UCHAR ucExtContrastLevel;
    UCHAR ucExtSaturationLevel;
    UCHAR ucExtColorSpaceMode;
    UCHAR ucExtYcDelay;
    UCHAR ucExtCueCorrect;
    UCHAR ucExtBorderLevel;
    UCHAR ucExtDeinterlaceMode;
    UCHAR ucExtNoiseReductionLevel;
    UCHAR ucExtEdgeEnhancementLevel;
    UCHAR ucExtDetailEnhancementLevel;
    UINT32 u4ZoomImage;
    UCHAR ucExtGamma;
    UINT8 u1HdmiRxInModeOn;
    UINT8 u1HdmiRxInBoardExist;
    UINT8 u1HdmiI2sMclk; 
}   HDMI_AV_INFO_T;



typedef BOOL (*Hdmi_avd_nfy_fct) (
HDMI_AVD_COND_T e_nfy_cond,
VOID*        pv_nfy_info,
SIZE_T*        pz_nfy_info_len);

//RM set notify function
#if UNIFORM_DRV_CALLBACK //new format for Linux
extern void vScomSetNfyFunc(DRV_CB_REG_INFO_T*  prNfyInfo);
#else 
extern void vScomSetNfyFunc(x_avd_nfy_fct  prNfyInfo);
#endif

extern void vScomGetEdidInfo(HDMI_EDID_INFO_T *pv_get_info);

// *********************************************************************
// Constant definitions
// *********************************************************************

#define AV_D_OK	     (INT32)(0)
#define AV_D_FAIL     (INT32)(-1)

//Speaker config for audio driver
//Speaker_config definition
/***************************************************
0: LT/RT
1: Mono
2: Stereo
3: L/R/C
4: L/R/S
5: L/R/C/S
6. L/R/LS/RS
7: L/R/C/LS/RS

bit 3: ch6 exist or not
bit 4: ch7 exist or not
bit 5: subwoofer exist or not
******************************************************************/
#define CH2_SPK_CONFIG  0x02
#define CH2_1_SPK_CONFIG  0x22
#define CH3_SPK_CONFIG  0x03
#define CH3_1_SPK_CONFIG  0x23
#define CH4_SPK_CONFIG  0x06
#define CH4_1_SPK_CONFIG  0x26
#define CH5_SPK_CONFIG  0x07
#define CH5_1_SPK_CONFIG  0x27
#define CH6_SPK_CONFIG  0x0F
#define CH6_1_SPK_CONFIG  0x2F
#define CH7_SPK_CONFIG  0x1F
#define CH7_1_SPK_CONFIG  0x3F

//For HDMI spdif audio raw data Type
#define SD_IEC_RAW_TYPE 0
#define HD_IEC_RAW_TYPE 1

//This HDMI_SINK_VIDEO_RES_T will define what kind of 
//video resolution can be supported by sink.

#define SINK_480P      (1<< 0)
#define SINK_720P60    (1<< 1)
#define SINK_1080I60   (1<< 2)
#define SINK_1080P60   (1<< 3)
#define SINK_480P_1440 (1<< 4)
#define SINK_480P_2880 (1<< 5)
#define SINK_480I      (1<< 6)//actuall 480Ix1440
#define SINK_480I_1440 (1<< 7)//actuall 480Ix2880
#define SINK_480I_2880 (1<< 8)//No this type for 861D
#define SINK_1080P30   (1<< 9)
#define SINK_576P      (1<< 10)
#define SINK_720P50    (1<< 11)
#define SINK_1080I50   (1<< 12)
#define SINK_1080P50   (1<< 13)
#define SINK_576P_1440 (1<< 14)
#define SINK_576P_2880 (1<< 15)
#define SINK_576I      (1<< 16)
#define SINK_576I_1440 (1<< 17)
#define SINK_576I_2880 (1<< 18)
#define SINK_1080P25   (1<< 19)
#define SINK_1080P24   (1<< 20)
#define SINK_1080P23976   (1<< 21)
#define SINK_1080P2997   (1<< 22)

//This HDMI_SINK_VIDEO_COLORIMETRY_T will define what kind of YCBCR 
//can be supported by sink. 
//And each bit also defines the colorimetry data block of EDID.
#define SINK_YCBCR_444 (1<<0)
#define SINK_YCBCR_422 (1<<1)
#define SINK_XV_YCC709 (1<<2)
#define SINK_XV_YCC601 (1<<3)
#define SINK_METADATA0 (1<<4)
#define SINK_METADATA1 (1<<5)
#define SINK_METADATA2 (1<<6)
#define SINK_RGB       (1<<7)


//HDMI_SINK_VCDB_T Each bit defines the VIDEO Capability Data Block of EDID. 
#define SINK_CE_ALWAYS_OVERSCANNED                  (1<<0)
#define SINK_CE_ALWAYS_UNDERSCANNED                 (1<<1)
#define SINK_CE_BOTH_OVER_AND_UNDER_SCAN            (1<<2)
#define SINK_IT_ALWAYS_OVERSCANNED                  (1<<3)
#define SINK_IT_ALWAYS_UNDERSCANNED                 (1<<4)
#define SINK_IT_BOTH_OVER_AND_UNDER_SCAN            (1<<5)
#define SINK_PT_ALWAYS_OVERSCANNED                  (1<<6)
#define SINK_PT_ALWAYS_UNDERSCANNED                 (1<<7)
#define SINK_PT_BOTH_OVER_AND_UNDER_SCAN            (1<<8)
#define SINK_RGB_SELECTABLE                         (1<<9)


//HDMI_SINK_AUDIO_DECODER_T define what kind of audio decoder 
//can be supported by sink.
#define   HDMI_SINK_AUDIO_DEC_LPCM        (1<<0)
#define   HDMI_SINK_AUDIO_DEC_AC3         (1<<1)
#define   HDMI_SINK_AUDIO_DEC_MPEG1       (1<<2)
#define   HDMI_SINK_AUDIO_DEC_MP3         (1<<3)
#define   HDMI_SINK_AUDIO_DEC_MPEG2       (1<<4) 
#define   HDMI_SINK_AUDIO_DEC_AAC         (1<<5)  
#define   HDMI_SINK_AUDIO_DEC_DTS         (1<<6)
#define   HDMI_SINK_AUDIO_DEC_ATRAC       (1<<7)
#define   HDMI_SINK_AUDIO_DEC_DSD         (1<<8) 
#define   HDMI_SINK_AUDIO_DEC_DOLBY_PLUS   (1<<9)
#define   HDMI_SINK_AUDIO_DEC_DTS_HD      (1<<10)
#define   HDMI_SINK_AUDIO_DEC_MAT_MLP     (1<<11)
#define   HDMI_SINK_AUDIO_DEC_DST         (1<<12)
#define   HDMI_SINK_AUDIO_DEC_WMA         (1<<13)


//Sink audio channel ability for a fixed Fs
#define SINK_AUDIO_2CH   (1<<0)
#define SINK_AUDIO_3CH   (1<<1)
#define SINK_AUDIO_4CH   (1<<2)
#define SINK_AUDIO_5CH   (1<<3)
#define SINK_AUDIO_6CH   (1<<4)
#define SINK_AUDIO_7CH   (1<<5)
#define SINK_AUDIO_8CH   (1<<6)

//Sink supported sampling rate for a fixed channel number
#define SINK_AUDIO_32k (1<<0)
#define SINK_AUDIO_44k (1<<1)
#define SINK_AUDIO_48k (1<<2)
#define SINK_AUDIO_88k (1<<3)
#define SINK_AUDIO_96k (1<<4)
#define SINK_AUDIO_176k (1<<5)
#define SINK_AUDIO_192k (1<<6)

//The following definition is for Sink speaker allocation data block .
#define SINK_AUDIO_FL_FR   (1<<0)
#define SINK_AUDIO_LFE     (1<<1)
#define SINK_AUDIO_FC      (1<<2)
#define SINK_AUDIO_RL_RR   (1<<3)
#define SINK_AUDIO_RC      (1<<4)
#define SINK_AUDIO_FLC_FRC (1<<5)
#define SINK_AUDIO_RLC_RRC (1<<6)

//The following definition is 
//For EDID Audio Support, //HDMI_EDID_CHKSUM_AND_AUDIO_SUP_T
#define SINK_BASIC_AUDIO_NO_SUP    (1<<0)
#define SINK_SAD_NO_EXIST          (1<<1)//short audio descriptor
#define SINK_BASE_BLK_CHKSUM_ERR   (1<<2)
#define SINK_EXT_BLK_CHKSUM_ERR    (1<<3)


//The following definition is for the output channel of 
//audio decoder AUDIO_DEC_OUTPUT_CHANNEL_T
#define AUDIO_DEC_FL   (1<<0)
#define AUDIO_DEC_FR   (1<<1)
#define AUDIO_DEC_LFE  (1<<2)
#define AUDIO_DEC_FC   (1<<3)
#define AUDIO_DEC_RL   (1<<4)
#define AUDIO_DEC_RR   (1<<5)
#define AUDIO_DEC_RC   (1<<6)
#define AUDIO_DEC_FLC  (1<<7)
#define AUDIO_DEC_FRC  (1<<8)




// *********************************************************************
// Variable define definitions
// *********************************************************************



// *********************************************************************
// Export API
// *********************************************************************
extern INT32 AV_D_Init(void);
extern INT32 i4Avd_Uninit(UINT32 u4Case);
//extern INT32 MW_AV_D_Init(void);
extern INT32 _AV_HdmiSet(AV_D_HDMI_DRV_SET_TYPE_T  e_set_type,
					const VOID*     pv_set_info,
					SIZE_T          z_set_info_len) ;
					 
extern INT32 _AV_HdmiGet(AV_D_HDMI_DRV_GET_TYPE_T   e_get_type,
					 VOID*           pv_get_info,
					SIZE_T*         pz_get_info_len);					

extern BOOL fgPmxDrvSetAvd(
PMX_SET_AVD_COND_T e_nfy_cond,
void*        pv_nfy_info,
UINT32        u4InfoLen);

extern INT32 fgPmxDrvGetAvdInfo(
PMX_DRV_GET_AVD_TYPE_T  e_get_type,
VOID*           pv_get_info	);

extern BOOL fgAudioNotify(
AUDIO_NFY_AVD_COND_T e_nfy_cond,
VOID*        pv_nfy_info,
SIZE_T*        pz_nfy_info_len);

extern BOOL fgVdpDrvSetAvd(
VDP_SET_AVD_COND_T e_nfy_cond,
void*        pv_nfy_info,
UINT32        u4InfoLen);

//for audio driver
BOOL fgAudDrvSetISRC1(BYTE bISRCType,  BYTE bISRCStatus, UINT8 *prISRCData);
extern BOOL fgAUDChgOutCfg(
  void   *prAudSrcParam,
  void   *prAudOutParam,
  void   *prAudHdmiOutParam);
extern BOOL fgAudDrvSethdmiReady(void);
void vSendHdmiISRC1(UINT8 u1ISRCType, UINT8 u1ISRCStatus, UINT8 *pr_bData);
void vSendHdmiACP(UINT8 u1ACPType, UINT8 *pr_bData);
extern void vSendHdmiSPD(UINT8 *pr_bData);
extern void vSendGCPMute(void);
//For SRM service
extern void vHdcpSrmSetFunc(UINT32 u4ByteCount, UINT8 *prData);
extern void vHdcpSrmGetSignatureChkFunc(UINT8 *prData);
//For VDP set xvYCC
extern void vVDPDrvSetXVYccBitStream(BOOL fgXVYccOn, UINT8 u1GamutType);
extern void vVDPDrvUpdateXVYccBitStream(void   *prXvYccInf);
extern void vVdpSetAspectRatio(UINT8 u1SourceAR, UINT8 u1DisplayARMode);
//For Other Driver set HDMI
void vDrvSetHDMIVideoBlackOnOff(BOOL fgBlackOn);
void vDrvSetHDMIAudioOnOff(BOOL fgAudioOff);
//For Debug enable
void vEnableHdmiDebug(UINT32 u4MessageType);
void vDisableHdmiDebug(UINT32 u4MessageType);
BOOL fgIsHdmiDebug(UINT32 u4MessageType);
BOOL fgIsHdmiDebugClear(void);
void vSetHdmiTurnOffDelayTime(INT32 i4DelayTime);
void vSetDviTurnOffDelayTime(INT32 i4DelayTime);
//for quickstart mode
void vHDMIEnable(BOOL fgEnable);
//for hdmi_if.c
INT32 vGetSinkAvCap(HDMI_SINK_AV_CAP_T **prAvCap);
void vSetAvdAvInfPointer(HDMI_AV_INFO_T *prAvinf);
void vTmdsOffAndResetHdcp(void);
void vChangeVpll(BYTE bRes);
void vResetVpll(void);
void vChgHDMIVideoResolution(void);
void vChgHDMIAudioOutput(HDMI_AUDIO_CHG_FUNCTION_TYPE_T bTYPE);
void vHDCPInitAuth(void);
void vSetHdmiColorSpace(UINT8 bColorSpace);
void vSetHdmiContrastAndBrightness(INT16 iGContrast, INT16 iGBright, UINT8 u1GammaMode);
void vSetHdmiHueAndSaturation(INT16 iDegree, INT16 iSaturation);
void vSetHdmiSharpness(BYTE u1SharpnessMode);
void vSetHdmiAspectRatio(UINT8 bAspectRatio);
void vChgtoSoftNCTS(void);
void vDisableHDCP(UINT8 fgDisableHdcp);
void vSetAvdCallBackFun(Hdmi_avd_nfy_fct prFun);
INT32 i4GetHotPlugStatus(void);
INT32 i4GetEdidStatus(void);
void vSetHdmiTxAudioZeroData(void);
void vSetHdmiTxAudioNonZeroData(void);
void vDisableHdmiTxAudioOutput(void);
void vEnableHdmiTxAudioOutput(void);
UINT32 u4GetSinkProductId(void);
void vEnableChgResUseAvMute(void);
void vDisableChgResUseAvMute(void);
UINT32 u4GetHdmiLogLevelFromNorFlash(void);
INT32 i4WriteHdmiLogLevelToNorFlash(UINT32 *u4prDbgLevel);
void vSetNoEdidCheck(UINT8 u1NoEdidOn);
void vSetDeepColor36bits(UINT8 u1Deep36BitsOn);
void vSetDeepColor48bits(UINT8 u1Deep36BitsOn);
void vSet480P2880On(UINT8 u1480p2800On);
extern void vDrvHdmiUpdataContentType(BYTE btype);

//void vShowEdidInformation(void);//Move to drv_hdmi.h
BOOL fgIsHotPlugChgRes(void);
void vSendAvdNotifyScomResChgOK(void);
void vAvdAudioStatus(void);
PMX_VSYNC_MODE_T ePmxDrvGetAvdVSyncMode(void);
void vPmxDrvSet3DSync(HDMI_RESOLUTION_T u4HDRes);
void vPmxDrvPreSet3DSyncDelay(HDMI_RESOLUTION_T u4HDRes,BOOL fgPmxPreSetOn);
void vPmxDrvResetAvdVSyncMode(PMX_VSYNC_MODE_T VSYNC_MODE);

void vHdmiRxDrvSetAvd(HDMI_RX_SET_AVD_COND_T e_nfy_cond,void*  pv_nfy_info);
void vSetDisableCat6023(BOOL u1On);

BOOL fgIsTxDetectHotPlugIn(void);
void vSetAvdHdmiRxInMode(BOOL fgRxInOn);
void vSetHdmiDisableCcirIn(void);
extern void vPmxDrvSet3DSync(HDMI_RESOLUTION_T u4HDRes);
extern void vPmxDrvPreSet3DSyncDelay(HDMI_RESOLUTION_T u4HDRes,BOOL fgPmxPreSetOn);
extern void vSetReencodeInfo(BOOL bHDMIReencode,BOOL bSPDIFReencode,UINT32 u4Disc_Type);
extern void d2_avd_reencode_inform_setting(SO_NY_AP_TO_AVD_REENCODE_PARAMETER_T *prReEncodeInf);
extern void d2_avd_aud_out_priority(UINT8 u1AudOutPriority);
extern void d2_avd_bd_audio_setting(UINT8 u1BdAudSetting);
extern void d2_avd_dolby_digital_setting(UINT8 u1DolbySetting);
extern void d2_avd_dts_setting(UINT8 u1DtsSetting);
extern void d2_avd_dsd_setting(UINT8 u1DsdSetting);
extern void d2_avd_jpeg_play_setting(BOOL fgJpegPlayOn);
extern void d2_avd_gama_setting(UINT8 u1GammaMode);
extern void d2_avd_set_xvColor_setting(UINT8 u1xvColor);
extern void d2_avd_set_tv_type_setting(UINT8 u1TvType);
extern void d2_avd_bd_dvd_audio_output_setting(UINT8 u1OutputMode);
extern void d2_avd_aac_setting(UINT8 u1AACOutType);
extern void d2_avd_set_edid_delay_enable(BOOL fg_AvdEdidDelaySupport);
extern void d2_avd_set_hdmi_audio_mute(UINT8 u1Mute);

extern void vSetPhiliReencodeInfo(BOOL fgBDPType,BOOL fgReEncode, BOOL fgByPassMixerflag, BOOL fgSAorIA,UINT32  u4Disc_Type,BOOL fgHDMIExteDecode,BOOL fgIEC48Khz,UINT32  u4DigitalOutType);;
extern UINT8 vAudCheckDrvCur(void);
extern void vSetAOSDATAOutput(UINT32 u4data);
BOOL fgIsPhotoHdMode(void);
void vShowGammaMode(void);
extern void vDynamicTuneApll(AUD_DEC_APLL_FS_CHG u1AudRateCtrl);
extern void vAdaptiveTuneAPLL(AUD_APLL_CTL_MODE u1APLLCtlMode, UINT16 u2APLLSetp);
BOOL fgAudDrvSetAudioInOnOff(BOOL fgOn);
#endif /* _DRV_AV_D_H_ */
