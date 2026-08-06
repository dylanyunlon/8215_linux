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

#ifndef _X_AUDIN_H_
#define _X_AUDIN_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "x_typedef.h"
#include "x_common.h"
#include "drv_config.h"
#include "drv_def.h"
#include "x_drv_cb.h"
#ifdef __linux__
#include <media/atc/x_rm.h>
#include <media/atc/x_memtype.h>
#include <media/atc/x_audmhl_def.h>
#else
#include "x_rm.h"
#include "x_memtype.h"
#include "x_audmhl_def.h"
#endif // __linux__

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
/* Get operations */
#define AUDIN_GET_ALL_ATUS                 (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 0))
#define AUDIN_GET_USB_CH_NO                 (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 1))
#define AUDIN_GET_USB_CODEC                 (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 2))
#define AUDIN_GET_HDMIRX_AUDINFO                 (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 3))
#define AUDIN_GET_USB_SAMPLERATE            (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 4))
//#define AUDIN_GET_PAUSE_STATUS          (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 1))

#if 0
/* Set operations */
#define AUDIN_SET_INPUT_SWITCH       (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 1))
#define AUDIN_SET_ON                 (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 2))
#define AUDIN_SET_OFF                (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 3))
#define AUDIN_SET_UNINIT             (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 4))
#define AUDIN_SET_INIT               (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 5))
#define AUDIN_SET_SACD_MUTE          (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 6))

#endif

typedef enum
{
    AUDMHL_SET_INPUT_SWITCH = 0,
    AUDMHL_SET_ON,
    AUDMHL_SET_OFF,
    AUDMHL_SET_UNINIT,
    AUDMHL_SET_INIT
}AUDMHL_CTRL_PRIVATE;

/*-----------------------------------------------------------------------------
                    AUDIN Get  information
 ----------------------------------------------------------------------------*/

typedef enum
{
  AUDIN_LOCK = 0x00,
  AUDIN_UNLOCK = 0x01,
  AUDIN_UNKNOWN = 0xff
} AUDIN_LOCK_STATUS;


typedef enum
{
  AUDIN_PAUSE = 0x00,
  AUDIN_PLAY = 0x01
} AUDIN_PAUSE_STATUS;

typedef enum
{
  AUDIN_OFF = 0x00,
  AUDIN_ON  = 0x01
} AUDIN_ONOFF;

typedef enum
{
  AUDIN = 0x00,
  IPODIN  = 0x01,
  PARTYIN  = 0x02
} AUDIN_TYPE;

typedef enum
{
  AUDIN_DISABLE = 0x00,
  AUDIN_ENABLE  = 0x01
} AUDIN_DIGITAL_DETECT;

typedef enum
{
  AUDIN_NULL = 0x00, // Disable AudIn
  AUDIN_OPTICAL1 = 0x01,
  AUDIN_OPTICAL2 = 0x02,
  AUDIN_COAXIAL = 0x03,
  AUDIN_HDMI = 0x04,
  AUDIN_ANALOG1 = 0x05,  // Line In
  AUDIN_ANALOG2 = 0x06
} AUDIN_INPUTS;

#if  CONFIG_DRV_HDMI_RX
typedef enum
{
  AUDIN_HBR_NONE = 0, // 
  AUDIN_HBR_MAT, //
  AUDIN_HBR_DTSMA, //
}   AUDIN_HBR_AUDIO_T;

typedef   struct  _AUDIN_PARSING_INFO_T
{
  bool  fgAudinDSD;
  __u8 u1DSDChNum;
  bool  fgAudinRAW;  // 1: RAW , 0:PCN
  bool  fgAudinReOrder;  // 1: need to reorder
  AUDIN_HBR_AUDIO_T AudinHBRAudioType;  // 
  __u32 u4PsrPcmUintSize;
}   AUDIN_PARSING_INFO_T;

//Audio In Type : I2S Mode, Raw Data
typedef enum
{   
    HDMI_I2S= 0x01,
    SPDIF_RAW= 0x02
} HDMI_SPDIF_IN_TYPE_T;

#endif

//ACP Type definition
#define ACP_TYPE_GENERAL_AUDIO  0x00
#define ACP_TYPE_IEC60958                0x01
#define ACP_TYPE_DVD_AUDIO           0x02
#define ACP_TYPE_SACD                       0x03
#define ACP_LOST_DISABLE                  0xFF

/*-----------------------------------------------------------------------------
                AUDIN to Multiple Line In
----------------------------------------------------------------------------*/
//Audio In Type : Line In, SPDIF In, Multiple Line In
#if 0
typedef enum
{   
    AUDIN_COAXL= 0x01,
    AUDIN_OPT3= 0x03,
    AUDIN_OPT1= 0x04,
    AUDIN_OPT2= 0x05,
    AUDIN_HDMIRX= 0x06,
    AUDIN_LINEIN= 0x07,
    AUDIN_MICIN= 0x08,
    AUDIN_NONE= 0xFF
} AUDIO_IN_TYPE_T;
#endif

/*-----------------------------------------------------------------------------
               HDMI RX task  to  AUDIN 
----------------------------------------------------------------------------*/
typedef enum
{   
   HDMIRX_I2S_L_R = 0,         
   HDMIRX_I2S_LS_RS = 1,         
   HDMIRX_I2S_C_LFE = 2,         
   HDMIRX_I2S_RLS_RRS = 3         
} HDMI_I2S_IN_CH_MAP_T;


typedef enum
{   
   HDMIRX_I2S_CH0_ON = (1<<0),         
   HDMIRX_I2S_CH1_ON = (1<<1),                           
   HDMIRX_I2S_CH2_ON = (1<<2),         
   HDMIRX_I2S_CH3_ON = (1<<3)
} HDMI_I2S_CH_SEL_T;



typedef enum
{   
  CH_REFER_TO_STREAM =0,
  COUNT_2CH = 1,
  COUNT_3CH = 2,
  COUNT_4CH = 3, 
  COUNT_5CH = 4, 
  COUNT_6CH = 5, 
  COUNT_7CH = 6, 
  COUNT_8CH = 7
   
} HDMI_RX_AUDIO_CHANNEL_COUNT_T;


typedef enum
{   
  CODE_REFER_TO_STREAM =0,
  CODE_PCM = 1,
  CODE_AC3 = 2,
  CODE_MPEG1 = 3, 
  CODE_MP3 = 4, 
  CODE_MPEG2 = 5, 
  CODE_AAC = 6, 
  CODE_DTS = 7, 
  CODE_ATRAC = 8, 
  CODE_DSD = 9, 
  CODE_DD_PLUS = 10, 
  CODE_DTS_HD = 11, 
  CODE_MAT = 12, 
  CODE_DST = 13, 
  CODE_WMA_PRO = 14, 
  
   
} HDMI_RX_AUDIO_CODING_TYPE_T;


typedef enum
{   
  SAMPLE_SIZE_REFER_TO_STREAM =0,
  SAMPLE_SIZE_16BIT = 1,
  SAMPLE_SIZE_20BIT = 2,
  SAMPLE_SIZE_24BIT = 3, 
    
} HDMI_RX_SAMPLE_SIZE_TYPE_T;

typedef enum
{   
  SAMPLE_FREQ_REFER_TO_STREAM =0,
  SAMPLE_FREQ_32KHZ = 1,
  SAMPLE_FREQ_44KHZ = 2,
  SAMPLE_FREQ_48KHZ = 3, 
  SAMPLE_FREQ_88KHZ = 4, 
  SAMPLE_FREQ_96KHZ = 5, 
  SAMPLE_FREQ_176KHZ = 6, 
  SAMPLE_FREQ_192HZ = 7, 
   
} HDMI_RX_SAMPLE_FREQ_TYPE_T;

typedef enum
{   
  LEVEL_0DB =0,
  LEVEL_1DB = 1,
  LEVEL_2DB = 2,
  LEVEL_3DB = 3, 
  LEVEL_4DB = 4, 
  LEVEL_5DB = 5, 
  LEVEL_6DB = 6, 
  LEVEL_7DB = 7, 
  LEVEL_8DB = 7, 
  LEVEL_9DB = 7, 
  LEVEL_10DB = 7, 
  LEVEL_11DB = 7, 
  LEVEL_12DB = 7, 
  LEVEL_13DB = 7, 
  LEVEL_14DB = 7, 
  LEVEL_15DB = 7, 
  
   
} HDMI_RX_AUD_LEVEL_TYPE_T;

typedef enum
{
    RX_MCLK_128FS,
    RX_MCLK_192FS,
    RX_MCLK_256FS,
    RX_MCLK_384FS,
    RX_MCLK_512FS,
    RX_MCLK_768FS
}   RX_MCLK_FREQUENCY_T;

typedef union _HDMI_RX_Audio_InfoFrame {
    struct {
        __u8 Type;
        __u8 Ver;
        __u8 Len;
        __u8 AudioChannelCount:3;//HDMI_RX_AUDIO_CHANNEL_COUNT_T
        __u8 RSVD1:1;
        __u8 AudioCodingType:4; //see HDMI_RX_AUDIO_CODING_TYPE_T
        __u8 SampleSize:2;//HDMI_RX_SAMPLE_SIZE_TYPE_T
        __u8 SampleFreq:3;//HDMI_RX_SAMPLE_FREQ_TYPE_T
        __u8 Rsvd2:3;
        __u8 FmtCoding;//no use
        __u8 SpeakerPlacement;//HDMI_RX_SPEAKER_ALLOCATE_T
        __u8 Rsvd3:3;
        __u8 LevelShiftValue:4;//HDMI_RX_AUD_LEVEL_TYPE_T
        __u8 DM_INH:1;// 1:Down mix Prohibites, 0:Permitted down mix
    } info;
    struct {
        __u8 AUD_HB[3];
        __u8 AUD_DB[10];//Original raw audio inforframe data (for other module it sould not refer to it)
    } pktbyte;
} HDMI_RX_Audio_InfoFrame;

typedef   struct  _HDMI_RX_IN_AUDIO_INFO_T
{

  __u8 u1HBRAudio; //  1: Means HBR audio in, 0:SD audio in
  __u8 u1DSDAudio;////  1: Means DSD audio in, 0:Non DSD audio in
  __u8 u1RawSDAudio; // 1: AC3, DTS SD raw bit-stream in, 0:PCM or HBR audio in
  __u8 u1PCMMultiCh; // 1: PCM Multi channel in ,  0: PCM 2 channel in
                      // PS: When u1RawSDAudio is 0 and u1HBRAudio is 0, this parameter  
                      //will be valid, otherwise don\A1\A6t care it
  __u8 u1FsDec; // by IEC937 and IEC 958 Spec,
                //0x00:44.1khz, 0x02:48khz, 0x03:32khz, 0x08:88.2khz
                //0x0A:96khz, 0x0c:176khz, 0x0E:192khz, 0x09:768khz(HBR)
  __u8 u1I2sChanValid; //bit0: I2S Channel 0  Valid
                        //bit1: I2S Channel 1  Valid  
                        //bit2: I2S Channel 2  Valid  
                        //bit3: I2S Channel 3  Valid  
                        
  __u8 u1I2sCh0Sel; // I2S channel 0 mapping: see HDMI_I2S_IN_CH_MAP_T
  __u8 u1I2sCh1Sel; // I2S channel 1 mapping: see HDMI_I2S_IN_CH_MAP_T
  __u8 u1I2sCh2Sel; // I2S channel 2 mapping: see HDMI_I2S_IN_CH_MAP_T
  __u8 u1I2sCh3Sel; // I2S channel 3 mapping: see HDMI_I2S_IN_CH_MAP_T
  RX_MCLK_FREQUENCY_T 	u1MCLKRatio;  // Show HDMI Rx mclk
}HDMI_RX_IN_AUDIO_INFO_T;

#if  CONFIG_DRV_HDMI_RX
typedef   struct _HDMI_RX_AUDIO_INFO_ALL_T
{
  HDMI_RX_AUDIO_FORMAT_T  u1HDMIRxAudFmt ;  // HDMI Rx audio format 
  HDMI_RX_PCM_INFO_T u4HDMIIRxPCMInfo;        // HDMI Rx PCM info
  HDMI_RX_AUDIO_CHSTS  u8HDMIRxAudCHSTS ; // HDMI Rx audio channel status
}
HDMI_RX_AUDIO_INFO_ALL_T;
#endif	

typedef enum
{   
  	
 HDMI_RX_AUDIO_ON =1 , 
 HDMI_RX_AUDIO_MUTE , 
 HDMI_RX_AUDIO_UNMUTE, 
 HDMI_RX_AUDIO_UNLOCK, 
 HDMI_RX_AUDIO_BIT_STREAM_CHANGE,
 HDMI_RX_AUDIO_INFORFRAME_CHANGE, 
//#if  CONFIG_DRV_HDMI_RX
 HDMI_RX_PLUG_OUT,
//#endif
 HDMI_RX_ACPPKT_CHG,
 HDMI_RX_AUDIO_CHG_PAUSE_STATUS,
}HDMI_RX_AUDIO_INT_TYPE;

typedef struct _HDMI_REG_AUD_F_{
	void  (*enableRxaud)(UINT8);
	void  (*getRxaudinfo)(HDMI_RX_IN_AUDIO_INFO_T*);
	int   (*getRxchannel)(HDMI_RX_AUDIO_CHSTS*);
	int   (*getRxaudinfoframe)(HDMI_RX_Audio_InfoFrame*);
	__u8 (*getRxacptype)(void);
	bool fgReg;
} HDMI_REG_AUD_F;

bool RegHDMIAudFunc(HDMI_REG_AUD_F *pRegFun);
void Audmhl_Reg_ForDemuxer(VOID *pvDmxTag,  VOID *pCallbacks);	
void Audmhl_UnReg_ForDemuxer(VOID);

/* Notify function */

#if (UNIFORM_DRV_CALLBACK)//for New Linux driver
typedef   struct  _AUDIN_CB_INFO_T
{
 AUDIN_COND_T e_audin_nfy_cond;
 __u8 u1AUDInNfyInfo;

}  AUDIN_CB_INFO_T;

#else

typedef VOID (*x_audin_nfy_fct) (
  AUDIN_COND_T e_nfy_cond,
  __u8        pv_nfy_info);

#endif



#endif
