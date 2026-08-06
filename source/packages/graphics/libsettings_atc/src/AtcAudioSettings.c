/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of AutoChips Inc. (C) 2013 AutoChips Inc.
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
*
* Filename:
* ---------
* file AtcAudioSettings.c
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*
*
* Author:
* -------
*
*
*------------------------------------------------------------------------------
* $Revision: #22 $
* $Modtime:$ 2015-09-02
* $Log:$
*
*******************************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#include "windev.h"
//#include "x_aud_dec.h"
//#include "aud_ioctrl.h"
//#include "aud_output.h"

#include "AtcAudioSettings.h"
#include "policyEvent.h"

#define AUD_DEV_NAME                    "/dev/adec"
#define PCM_DEV                         "/dev/pcm_aud"

#define AUD_SE_OPCODE_OFF       (1 << 0)
#define AUD_SE_OPCODE_ON        (1 << 1)
#define AUD_SE_OPCODE_UPG_COEF  (1 << 2)
#define AUD_SE_OPCODE_CTRL      (1 << 4)
#define AUD_SE_CSII_SET_LEVEL   (1 << 7)


/*------------------Speaker Layout----------------*/
//Speaker Layout format define
//bit0~bit2
#define SPK_LAYOUT_LR                      0X0 //000
#define SPK_LAYOUT_MONO                    0X1 //001
#define SPK_LAYOUT_STEREO                  0X2 //010
#define SPK_LAYOUT_LRC                     0X3 //011
#define SPK_LAYOUT_LRS                     0X4 //100
#define SPK_LAYOUT_LRCS                    0X5 //101
#define SPK_LAYOUT_LRLSRS                  0X6 //110
#define SPK_LAYOUT_LRCLSRS                 0X7 //111

#define DEFAULT_VALUE_BRIGHTNESS                        0x32
#define DEFAULT_VALUE_CONTRAST                          0x14
#define DEFAULT_VALUE_BACKLIGHT_LEVEL                   0x37
#define DEFAULT_VALUE_HUE                               0x32
#define DEFAULT_VALUE_SATURATION                        0x32
#define DEFAULT_VALUE_BACKLIGHT                         0x37
#define DEFAULT_VALUE_MUTE                              0x07
#define DEFAULT_VALUE_VOLUME                            0x32
#define DEFAULT_VALUE_REARVOLUME                        0x32
#define DEFAULT_VALUE_EQBAND0                           0x00
#define DEFAULT_VALUE_EQBAND1                           0x00
#define DEFAULT_VALUE_EQBAND2                           0x00
#define DEFAULT_VALUE_EQBAND3                           0x00
#define DEFAULT_VALUE_EQBAND4                           0x00
#define DEFAULT_VALUE_EQBAND5                           0x00
#define DEFAULT_VALUE_EQBAND6                           0x00
#define DEFAULT_VALUE_EQBAND7                           0x00
#define DEFAULT_VALUE_EQBAND8                           0x00
#define DEFAULT_VALUE_EQBAND9                           0x00
#define DEFAULT_VALUE_EQBAND10                          0x00
#define DEFAULT_VALUE_TESTTONE                          0x02
#define DEFAULT_VALUE_FLEFT                             0x14
#define DEFAULT_VALUE_FRIGHT                            0x14
#define DEFAULT_VALUE_RLEFT                             0x14
#define DEFAULT_VALUE_RRIGHT                            0x14
#define DEFAULT_VALUE_CENTER                            0x14
#define DEFAULT_VALUE_SUBWOOFER                         0x14
#define DEFAULT_VALUE_SRSSWITCH                         0x00
#define DEFAULT_VALUE_SRSMODE                           0x00
#define DEFAULT_VALUE_SRSPHANTOM                        0x00
#define DEFAULT_VALUE_SRSFULLBAND                       0x00
#define DEFAULT_VALUE_FOCUS0                            0x00
#define DEFAULT_VALUE_FOCUS1                            0x00
#define DEFAULT_VALUE_FOCUS2                            0x00
#define DEFAULT_VALUE_SRSTB                             0x00
#define DEFAULT_VALUE_TBSS0                             0x01
#define DEFAULT_VALUE_TBSS1                             0x01
#define DEFAULT_VALUE_TBSS2                             0x01
#define DEFAULT_VALUE_SPEAKERLAYOUT                     0x01
#define DEFAULT_VALUE_SPEAKERSIZE                       0x06
#define DEFAULT_VALUE_EQTYPE                            0x00
#define DEFAULT_VALUE_PL2TYPE                           0x00
#define DEFAULT_VALUE_REVERBTYPE                        0x00
#define DEFAULT_VALUE_UPMIX                             0x00
#define DEFAULT_VALUE_LOUDNESS                          0x00
#define DEFAULT_VALUE_CHANNELTYPE                       0x00

#define DEFAULT_CLIENTVALUE_VOLUME                      0x10000
#define DEFAULT_CLIENTVALUE_REARVOLUME                  0x10000
#define DEFAULT_CLIENTVALUE_FLEFT                       0x20000
#define DEFAULT_CLIENTVALUE_FRIGHT                      0x20000
#define DEFAULT_CLIENTVALUE_RLEFT                       0x20000
#define DEFAULT_CLIENTVALUE_RRIGHT                      0x20000
#define DEFAULT_CLIENTVALUE_CENTER                      0x20000
#define DEFAULT_CLIENTVALUE_SUBWOOFER                   0x20000
#define DEFAULT_CLIENTVALUE_TESTTONE                    0x02
#define DEFAULT_CLIENTVALUE_TESTTONE_TYPE               0x00
#define DEFAULT_CLIENTVALUE_UPMIX_TYPE                  0x00
#define DEFAULT_CLIENTVALUE_UPMIX_GAIN0                 0x00
#define DEFAULT_CLIENTVALUE_UPMIX_GAIN1                 0x00
#define DEFAULT_CLIENTVALUE_UPMIX_GAIN2                 0x00
#define DEFAULT_CLIENTVALUE_UPMIX_GAIN3                 0x00
#define DEFAULT_CLIENTVALUE_UPMIX_GAIN4                 0x00
#define DEFAULT_CLIENTVALUE_UPMIX_GAIN5                 0x00
#define DEFAULT_CLIENTVALUE_UPMIX_GAIN6                 0x00
#define DEFAULT_CLIENTVALUE_UPMIX_GAIN7                 0x00
#define DEFAULT_CLIENTVALUE_LOUDNESS_TYPE               0x00
#define DEFAULT_CLIENTVALUE_LOUDNESS_GAIN0              0x00
#define DEFAULT_CLIENTVALUE_LOUDNESS_GAIN1              0x00
#define DEFAULT_CLIENTVALUE_LOUDNESS_GAIN2              0x00
#define DEFAULT_CLIENTVALUE_LOUDNESS_GAIN3              0x00
#define DEFAULT_CLIENTVALUE_LOUDNESS_GAIN4              0x00
#define DEFAULT_CLIENTVALUE_LOUDNESS_GAIN5              0x00
#define DEFAULT_CLIENTVALUE_REVERB_TYPE                 0x00
#define DEFAULT_CLIENTVALUE_REVERB_GAIN                 0x180000
#define DEFAULT_CLIENTVALUE_REVERB_FEEDBACK_GAIN        0x7FFFFF
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE00           0x29
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE01           0x1F
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE02           0x17
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE03           0x0D
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE10           0x29
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE11           0x1F
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE12           0x17
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE13           0x0D
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE20           0x29
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE21           0x1F
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE22           0x17
#define DEFAULT_CLIENTVALUE_REVERB_BANKSIZE23           0x0D
#define DEFAULT_CLIENTVALUE_EQSET                       0x00
#define DEFAULT_CLIENTVALUE_EQ_GAIN0                    0x10000
#define DEFAULT_CLIENTVALUE_EQ_GAIN1                    0x00
#define DEFAULT_CLIENTVALUE_EQ_GAIN2                    0x00
#define DEFAULT_CLIENTVALUE_EQ_GAIN3                    0x00
#define DEFAULT_CLIENTVALUE_EQ_GAIN4                    0x00
#define DEFAULT_CLIENTVALUE_EQ_GAIN5                    0x00
#define DEFAULT_CLIENTVALUE_EQ_GAIN6                    0x00
#define DEFAULT_CLIENTVALUE_EQ_GAIN7                    0x00
#define DEFAULT_CLIENTVALUE_EQ_GAIN8                    0x00
#define DEFAULT_CLIENTVALUE_EQ_GAIN9                    0x00
#define DEFAULT_CLIENTVALUE_EQ_GAIN10                   0x00
#define DEFAULT_CLIENTVALUE_EQTYPE                      0x00
#define DEFAULT_CLIENTVALUE_EQTYPE_GAIN0                0x10000
#define DEFAULT_CLIENTVALUE_EQTYPE_GAIN1                0x00
#define DEFAULT_CLIENTVALUE_EQTYPE_GAIN2                0x00
#define DEFAULT_CLIENTVALUE_EQTYPE_GAIN3                0x00
#define DEFAULT_CLIENTVALUE_EQTYPE_GAIN4                0x00
#define DEFAULT_CLIENTVALUE_EQTYPE_GAIN5                0x00
#define DEFAULT_CLIENTVALUE_EQTYPE_GAIN6                0x00
#define DEFAULT_CLIENTVALUE_EQTYPE_GAIN7                0x00
#define DEFAULT_CLIENTVALUE_EQTYPE_GAIN8                0x00
#define DEFAULT_CLIENTVALUE_EQTYPE_GAIN9                0x00
#define DEFAULT_CLIENTVALUE_EQTYPE_GAIN10               0x00
#define DEFAULT_CLIENTVALUE_MVS_TYPE                    0x00
#define DEFAULT_CLIENTVALUE_GAIN_SCALE                  0x00
#define DEFAULT_CLIENTVALUE_WIDTH_GAIN                  0x00
#define DEFAULT_CLIENTVALUE_LR_GAIN                     0x00
#define DEFAULT_CLIENTVALUE_CENTER_GAIN                 0x00
#define DEFAULT_CLIENTVALUE_CROSSTALK_GAIN              0x00
#define DEFAULT_CLIENTVALUE_BASS_GAIN                   0x00
#define DEFAULT_CLIENTVALUE_OUTPUT_GAIN                 0x00
#define DEFAULT_CLIENTVALUE_INPUT_GAIN                  0x00
#define DEFAULT_CLIENTVALUE_VRPHASE                     0x00


//bit5 subwoofer exist or not
#define SPK_LAYOUT_SUBWOOFER               0X0000000000000001LL << 5 

//bit12 Center channel large or small
#define SPK_LAYOUT_C_LARGE                 0X0000000000000001LL << 12

//bit13 Left channel large or small
#define SPK_LAYOUT_L_LARGE                 0X0000000000000001LL << 13

//bit 14 Right channel large or small
#define SPK_LAYOUT_R_LARGE                 0X0000000000000001LL << 14

//bit 15 Left surround channel large or small
#define SPK_LAYOUT_LS_LARGE                0X0000000000000001LL << 15

//bit 16 Right surround channel large or small
#define SPK_LAYOUT_RS_LARGE                0X0000000000000001LL << 16

//bit 32 Center exist or not
#define SPK_LAYOUT_C_EXIST                 0X0000000000000001LL << 32

//bit33 LR exist or not
#define SPK_LAYOUT_LR_EXIST                0X0000000000000001LL << 33

//bit 34 LS/RS exist or not 
#define SPK_LAYOUT_LSRS_EXIST              0X0000000000000001LL << 34

//bit 35 LFE exist or not
#define SPK_LAYOUT_LFE_EXIST               0X0000000000000001LL << 35

#define MAX_LEVEL 29

#define EQBAND_NUM             11

#define AUD_IOCTRL_ID_START         0x800

#define DEFINE_AUD_IOCTRL(ID)                       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, ID, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_AUDIO_SET_VOLUME                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x06)

#define IOCTL_AUDIO_SET_SE                          \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x13)

#define IOCTL_AUDIO_SET_DIVERSITY_INFO              \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x15)

#define IOCTL_AUDIO_SET_FEATURE                     \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x18)

#define IOCTL_AUDIO_SET_SPEAKER_LAYOUT              \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x23)

#define IOCTL_AUDIO_AOUT_CONFIG                     \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x27)

#define IOCTL_AUDIO_SET_MUTE_TYPE                    \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x28)

#define IOCTL_AUD_SET_TEST_TONE_TYPE      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x29)   

#define IOCTL_AUD_SET_TEST_TONE_CHANNEL      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x2A)   

#define IOCTL_AUD_SET_TEST_TONE_ONOFF      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x2B)  

#define IOCTL_AUD_SET_REAR_VOLUME              \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x2D)

#define IOCTL_AUD_SPDIF_ENABLE                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x2E)
    
#define IOCTL_AUD_SET_DAC_TYPE                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x2F)

#define IOCTL_AUD_GET_FRONT_STATUS                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x30)

#define IOCTL_AUD_GET_REAR_STATUS                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x31)

#define IOCTL_AUD_SET_LRMIX                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x32)

#define IOCTL_AUD_FUNC_OPTION_SET                             \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x33)

#define IOCTL_AUD_SET_REAR_I2S_GROUP                             \
        DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x36) 

#define IOCTL_AUD_SET_THRESHOLD                       \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x38)
    
#define IOCTL_AUD_SET_TYPE_SPDIF                       \
        DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x39)

#define IOCTL_AUDIO_SET_CH_DELAY                       \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x3B)

#define IOCTL_AUDIO_SET_VOL_POLICY                         \
        DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x80)



#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#define IOCTL_HAL_GET_CHIP_FEATURE             CTL_CODE (FILE_DEVICE_UNKNOWN, 2106, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_SET_TOUCH_DIS_WORK               CTL_CODE(FILE_DEVICE_UNKNOWN, 0x356, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define PCM_AUD_IOCTRL_ID_START			0x600

#define DEFINE_PCM_AUD_IOCTRL(ID)				\
	CTL_CODE(FILE_DEVICE_UNKNOWN, (ID), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define SET_SPH_DELAY							\
	DEFINE_PCM_AUD_IOCTRL(PCM_AUD_IOCTRL_ID_START + 0x00)

#define GET_SPH_DELAY							\
	DEFINE_PCM_AUD_IOCTRL(PCM_AUD_IOCTRL_ID_START + 0x01)

#define SET_SPH_MIC_GAIN						\
	DEFINE_PCM_AUD_IOCTRL(PCM_AUD_IOCTRL_ID_START + 0x05)

#define GET_SPH_MIC_GAIN						\
	DEFINE_PCM_AUD_IOCTRL(PCM_AUD_IOCTRL_ID_START + 0x06)

#define SET_BT_SPH_GAIN							\
	DEFINE_PCM_AUD_IOCTRL(PCM_AUD_IOCTRL_ID_START + 0x33)

#define SET_DEVICE_SPH_GAIN						\
	DEFINE_PCM_AUD_IOCTRL(PCM_AUD_IOCTRL_ID_START + 0x09)

#define SET_DSP_MIX_CH         \
    DEFINE_PCM_AUD_IOCTRL(PCM_AUD_IOCTRL_ID_START + 0x0B)

#define SET_MIC_MUTE         \
    DEFINE_PCM_AUD_IOCTRL(PCM_AUD_IOCTRL_ID_START + 0x44)

#define SET_PRIMARY_MIC							\
	DEFINE_PCM_AUD_IOCTRL(PCM_AUD_IOCTRL_ID_START + 0x45)




#define AUD_SE_OPCODE_OFF       (1 << 0)
#define AUD_SE_OPCODE_ON        (1 << 1)
#define AUD_SE_OPCODE_UPG_COEF  (1 << 2)
#define AUD_SE_OPCODE_CTRL      (1 << 4)
#define AUD_SE_CSII_SET_LEVEL   (1 << 7)


#define AUD_DEC_EQ_SET_U4ENABLE      (1<<0)
#define AUD_DEC_EQ_SET_U4BANDNUM     (1<<1)
#define AUD_DEC_EQ_SET_AU4GAIN       (1<<2)
#define AUD_DEC_EQ_SET_IIR_COEF      (1<<3)



static __u32 g_ganValues[MAX_LEVEL]={
    0xFFFF3315,0xFFFF3950,0xFFFF404E,0xFFFF4827,0xFFFF50F5,0xFFFF5AD6,0xFFFF65EB,
    0xFFFF725A,0xFFFF804E,0xFFFF8FF6,0xFFFFA187,0xFFFFB53C,0xFFFFCB5A,0xFFFFE42A,
    0x00000000,0x00001F3C,0x00004248,0x0000699C,0x000095BB,0x0000C73D,0x0000FEC9,
    0x00013D1C,0x0001830A,0x0001D181,0x0002298B,0x00028C52,0x0002FB27,0x00037782,
    0x0004030A
};

typedef struct _SYS_SET_VALUES
{
    AUD_MUTE_TYPE_T             eMute;
    __u32                     u4Volume;
    __u32                     u4RearVolume;
    AUD_EQVALUES_T              rEQValues;
    AUD_TEST_TONE_T             eTestTone;
    __s32                      i4BalanceFL;
    __s32                      i4BalanceFR;
    __s32                      i4BalanceRL;
    __s32                      i4BalanceRR;
    __s32                      i4BalanceC;
    __s32                      i4BalanceSub;
    MISC_AUD_SE_CSII_SWITCH_T   eSRSSwitch;
    MISC_AUD_SE_CSII_MODE_T     eSRSMode;
    MISC_AUD_SE_CSII_SWITCH_T   eSRSPhantom;
    MISC_AUD_SE_CSII_SWITCH_T   eSRSFB;
    MISC_AUD_SE_CSII_SWITCH_T   eFocusC;
    MISC_AUD_SE_CSII_SWITCH_T   eFocusF;
    MISC_AUD_SE_CSII_SWITCH_T   eFocusR;
    MISC_AUD_SE_CSII_SWITCH_T   eSRSTB;
    MISC_AUD_SE_CSII_SS_T       eTBSSF;
    MISC_AUD_SE_CSII_SS_T       eTBSSSub;
    MISC_AUD_SE_CSII_SS_T       eTBSSR;
    __u32                     u4SpeakerLayoutType;
    __u32                     u4SpeakerSize;
    EQTYPE_T                    eEQType;
    PLII_TYPE                   ePL2Type;
    REVERBTYPE_T                eReverbType;
    AUD_UPMIX_T                 eUpMixType;
    __u32                      uLoudNessType;
    AUD_DEC_LRMIX_OUTPUT_T      eChannelType;
    __u32                     u4CltVolume;
    __u32                     u4CltRearVolume;
    __u32                     u4CltBalanceFL;
    __u32                     u4CltBalanceFR;
    __u32                     u4CltBalanceRL;
    __u32                     u4CltBalanceRR;
    __u32                     u4CltBalanceC;
    __u32                     u4CltBalanceSub;
    AUD_TEST_TONE_T             eCltTestTone;
    MISC_TEST_TONE_TYPE_T       eCltTestToneType;
    AUD_UPMIX_T                 eCltUpMixType;
    MISC_UPMIX_GAIN_T           rCltUpmixGain;
    __u8                      uCltLoudNessType;
    MISC_LOUDNESS_GAIN_T        rCltLoudNessGain;
    REVERBTYPE_T                eCltReverbType;
    MISC_REVERB_COEF_T          rCltReverbCoef;
    __u8                       fgCltEQSet;
    MISC_EQ_GAIN_T              rCltEQGain;
    EQTYPE_T                    eCltEQType;
    MISC_EQ_GAIN_T              rCltEQTypeGain;
    MISC_MVS_T                  eCltMVSType;
    MISC_MVS_GAIN_T             rCltMVSGain;
 } SYS_SET_VALUES_T;

typedef struct _AUD_FUNC_OPTION
{
    __u32 u4FuncOption0;        //default: 0
    __u32 u4FuncOption1;        //default: 0
    __u32 u4FuncOption2;        //default: 0
    __u32 u4BassCutOffFreq;    //default: 100 (Hz)
    __u32 u4GainAvIn;            //default: 0x20000
    __u32 u4GainUSB;            //default: 0x20000
    __u32 u4GainDVD;            //default: 0x20000    
    __u32 u4Reserve1;            //default: 0
    __u32 u4Reserve2;            //default: 0
    __u32 u4Reserve3;            //default: 0
    __u32 u4Reserve4;            //default: 0
    __u32 u4Reserve5;            //default: 0
    __u32 u4Reserve6;            //default: 0    
}AUD_FUNC_OPTION_T;

typedef struct _AUD_THRESHOLD
{
    __u32 u4FrontThrshld;        
    __u32 u4RearThrshld;
    __u32 u4WaveFormThrshld;
}AUD_THRESHOLD_T;

/*-------------PLII-----------------------*/


/* PROLOGICII type */
typedef enum
{
    AUD_SE_PL2_CTRL_SWITCH = 0,
    AUD_SE_PL2_CTRL_MODE,
    AUD_SE_PL2_CTRL_PANORAMA,
    AUD_SE_PL2_CTRL_DIMENSION,
    AUD_SE_PL2_CTRL_C_WIDTH
}   AUD_SE_PL2_CTRL_T;


/* switch operation value */
typedef enum
{
    AUD_SE_PL2_SWITCH_OFF = 0,
    AUD_SE_PL2_SWITCH_ON,
    AUD_SE_PL2_SWITCH_AUTO
}   AUD_SE_PL2_SWITCH_T;

/* mode operation value */
typedef enum
{
    AUD_SE_PL2_MODE_PROLGIC_EMULATION = 0,
    AUD_SE_PL2_MODE_VIRTUAL,
    AUD_SE_PL2_MODE_MUSIC,
    AUD_SE_PL2_MODE_MOVIE,
    AUD_SE_PL2_MODE_MATRIX,
    AUD_SE_PL2_MODE_CUSTOM
}   AUD_SE_PL2_MODE_T;

/* Panorama operation value */
typedef enum
{
    AUD_SE_PL2_PANORAMA_OFF = 0,
    AUD_SE_PL2_PANORAMA_ON

}   AUD_SE_PL2_PANORAMA_T;

typedef struct AUD_SE_PL2_VAL_MIN_MAX_T
{
    __u8      ui1_curr_val;
    __u8      ui1_min_val; /* Only used in AUD_DRV_GET_TYPE_PL2_CTRL */
    __u8      ui1_max_val; /* Only used in AUD_DRV_GET_TYPE_PL2_CTRL */
} AUD_SE_PL2_VAL_MIN_MAX_T;


/* PROLOGICII type */
typedef struct AUD_SE_PL2_CTRL_INFO_T
{
    AUD_SE_PL2_CTRL_T    e_ctrlID; /* IN */

    union
    {
        AUD_SE_PL2_SWITCH_T         e_pl2_switch;
        AUD_SE_PL2_MODE_T           e_pl2_mode;
        AUD_SE_PL2_PANORAMA_T       b_is_pl2_panorama_on;
        AUD_SE_PL2_VAL_MIN_MAX_T    t_pl2_dim_val;
        AUD_SE_PL2_VAL_MIN_MAX_T    t_pl2_c_width_val;
    } u_value;

} AUD_SE_PL2_CTRL_INFO_T;

/*------------------Audio SRS----------------*/
typedef enum
{
    AUD_SE_CSII_SWITCH_OFF = 0,
    AUD_SE_CSII_SWITCH_ON,
    AUD_SE_CSII_SWITCH_AUTO
}   AUD_SE_CSII_SWITCH_T;

typedef enum
{
    AUD_SE_CSII_PHANTOM_OFF = 0,
    AUD_SE_CSII_PHANTOM_ON,
}AUD_SE_CSII_PHANTOM_T;

typedef enum
{
    AUD_SE_CSII_FB_OFF = 0,
    AUD_SE_CSII_FB_ON,
}AUD_SE_CSII_FB_T;

typedef enum
{
    AUD_SE_CSII_FOCUSC_OFF = 0,
    AUD_SE_CSII_FOCUSC_ON,
}AUD_SE_CSII_FOCUSC_T;

typedef enum
{
    AUD_SE_CSII_FOCUSF_OFF = 0,
    AUD_SE_CSII_FOCUSF_ON,
}AUD_SE_CSII_FOCUSF_T;

typedef enum
{
    AUD_SE_CSII_FOCUSR_OFF = 0,
    AUD_SE_CSII_FOCUSR_ON,
}AUD_SE_CSII_FOCUSR_T;

typedef enum
{
    AUD_SE_CSII_TB_OFF = 0,
    AUD_SE_CSII_TB_ON,
}AUD_SE_CSII_TB_T;    

typedef enum
{
    AUD_SE_CSII_MODE_CINEMA = 0,
    AUD_SE_CSII_MODE_PRO,
    AUD_SE_CSII_MODE_MUSIC,
    AUD_SE_CSII_MODE_MONO,
    AUD_SE_CSII_MODE_LCRS,
}   AUD_SE_CSII_MODE_T;

typedef enum
{
    AUD_SE_CSII_SS_40HZ  = (1 << 0), // Speaker size is 40Hz
    AUD_SE_CSII_SS_60HZ  = (1 << 1),
    AUD_SE_CSII_SS_100HZ = (1 << 2),
    AUD_SE_CSII_SS_150HZ = (1 << 3),
    AUD_SE_CSII_SS_200HZ = (1 << 4),
    AUD_SE_CSII_SS_250HZ = (1 << 5),
    AUD_SE_CSII_SS_300HZ = (1 << 6),
    AUD_SE_CSII_SS_400HZ = (1 << 7),
}   AUD_SE_CSII_SS_T;

typedef enum
{
    AUD_SE_CSII_F2R_0,
    AUD_SE_CSII_F2R_1,
} AUD_SE_CSII_F2R_T;

typedef enum
{
    AUD_SE_CSII_C2R_0,
    AUD_SE_CSII_C2R_1,
} AUD_SE_CSII_C2R_T;

typedef enum
{
    AUD_SE_CSII_CTRL_SWITCH = 0,
    AUD_SE_CSII_CTRL_MODE,
    AUD_SE_CSII_CTRL_PHANTOM,
    AUD_SE_CSII_CTRL_FB,
    AUD_SE_CSII_CTRL_FOCUS_CENTER,
    AUD_SE_CSII_CTRL_FOCUS_FRONT,
    AUD_SE_CSII_CTRL_FOCUS_REAR,
    AUD_SE_CSII_CTRL_TB,
    AUD_SE_CSII_CTRL_F_SS, // front speaker size
    AUD_SE_CSII_CTRL_S_SS, // sub speaker size
    AUD_SE_CSII_CTRL_R_SS, // rear speaker size
    AUD_SE_CSII_CTRL_F2R,
    AUD_SE_CSII_CTRL_C2R,
    AUD_SE_CSII_CTRL_TBF,
    AUD_SE_CSII_CTRL_TBS,
    AUD_SE_CSII_CTRL_TBR
}   AUD_SE_CSII_CTRL_T;

typedef struct AUD_SE_CSII_CTRL_INFO_TAG
{
    AUD_SE_CSII_CTRL_T    e_ctrlID; /* IN */

    union {
        AUD_SE_CSII_SWITCH_T        e_csii_switch;
        AUD_SE_CSII_MODE_T          e_mode;
        AUD_SE_CSII_PHANTOM_T       e_phantom;
        AUD_SE_CSII_FB_T            e_fb;
        AUD_SE_CSII_FOCUSC_T        e_focuscenter;
        AUD_SE_CSII_FOCUSF_T        e_focusfront;
        AUD_SE_CSII_FOCUSR_T        e_focusrear;
        AUD_SE_CSII_TB_T            e_TB;
        AUD_SE_CSII_SS_T            e_front_ss;
        AUD_SE_CSII_SS_T            e_sub_ss;
        AUD_SE_CSII_SS_T            e_rear_ss;
        AUD_SE_CSII_F2R_T           e_f2r;
        AUD_SE_CSII_C2R_T           e_c2r;
    } u;
} AUD_SE_CSII_CTRL_INFO_T;

typedef enum
{
    AUD_SE_CSII_FOCUS_CENTER_LEVEL = 16, //AUD_SE_CSII_CTRL_T already use 0~15.
    AUD_SE_CSII_FOCUS_FRONT_LEVEL,
    AUD_SE_CSII_FOCUS_REAR_LEVEL,
    AUD_SE_CSII_TRUBASS_FRONT_LEVEL,
    AUD_SE_CSII_TRUBASS_SUB_LEVEL,
    AUD_SE_CSII_TRUBASS_REAR_LEVEL,
    AUD_SE_CSII_FRONT2REAR_LEVEL,
    AUD_SE_CSII_CENTER2REAR_LEVEL
} AUD_SE_CSII_SPKID_T;


typedef struct AUD_SE_CSII_LEVEL_CTRL_INFO_TAG
{
    AUD_SE_CSII_SPKID_T e_LevelctrlID; /* IN */
    union {
        __s32 i4FocusCenterLevel;
        __s32 i4FocusFrontLevel;
        __s32 i4FocusRearLevel;
        __s32 i4TBFrontLevel;
        __s32 i4TBSubLevel;
        __s32 i4TBRearLevel;
        __s32 i4Front2RearLevel;
        __s32 i4Center2RearLevel;
    } u;
} AUD_SE_CSII_LEVEL_CTRL_INFO_T;


/*-------------------------------------------*/

typedef enum
{
    AUD_DEC_MUTE_OFF = 0,
    AUD_DEC_MUTE_ON
}AUD_DEC_MUTE_TYPE_T;

typedef enum
{
    AUD_DEC_ALL_CH = 0,
    AUD_DEC_INDIVIDUAL_CH
}   AUD_DEC_VOL_TYPE_T;

typedef enum
{
    AUD_DEC_OUT_PORT_OFF = 0,
    AUD_DEC_OUT_PORT_2_CH,
    AUD_DEC_OUT_PORT_5_1_CH,
    AUD_DEC_OUT_PORT_SPDIF,
    AUD_DEC_OUT_PORT_2_CH_BY_PASS,
    AUD_DEC_OUT_PORT_SPEAKER
}   AUD_DEC_OUT_PORT_T;

typedef enum
{
    AUD_DEC_LS_FRONT_LEFT = 0,
    AUD_DEC_LS_FRONT_RIGHT,
    AUD_DEC_LS_REAR_LEFT,
    AUD_DEC_LS_REAR_RIGHT,
    AUD_DEC_LS_CENTER,
    AUD_DEC_LS_SUB_WOOFER,
    AUD_DEC_LS_SPK_AUX1,                  //back left
    AUD_DEC_LS_SPK_AUX2,                  //back right
    AUD_DEC_LS_SPK_DOWNMIX_LEFT,
    AUD_DEC_LS_SPK_DOWNMIX_RIGHT,
    AUD_DEC_LS_SPK_ALL
}   AUD_DEC_LS_T;

typedef struct _AUD_DEC_CH_VOLUME_T
{
    AUD_DEC_OUT_PORT_T  e_out_port;
    AUD_DEC_LS_T        e_ls;

    __u8               ui1_level;
}   AUD_DEC_CH_VOLUME_T;

typedef struct _AUD_DEC_VOLUME_INFO_T
{
    AUD_DEC_VOL_TYPE_T            e_vol_type;

    union
    {
        __u8                   ui1_level;
        AUD_DEC_CH_VOLUME_T     t_ch_vol;
    } u;
}   AUD_DEC_VOLUME_INFO_T;

typedef struct _AUD_DEC_REAR_VOLUME_INFO_T
{    
    __u32    ui1_level;

}   AUD_DEC_REAR_VOLUME_INFO_T;

/* Individual channel volume gain setting. */
typedef struct _AUD_DEC_CH_VOLUME_GAIN_T
{
    AUD_DEC_OUT_PORT_T  e_out_port;
    AUD_DEC_LS_T        e_ls;

    __u32  u4FrontChVolGain;
}AUD_DEC_CH_VOLUME_GAIN_T;

typedef struct _AUD_DEC_VOLUME_GAN_INFO_T
{
    AUD_DEC_VOL_TYPE_T  e_vol_type;

    union
    {
        __u32    u4FrontMasterVolGain;
        AUD_DEC_CH_VOLUME_GAIN_T     t_ch_gain_vol;
    } u;
}AUD_DEC_VOLUME_GAIN_INFO_T;

typedef enum
{
    AUD_VOL_NORMAL = 0,
    AUD_VOL_SET_POLICY,
    AUD_VOL_RESET_POLICY
}AUD_VOL_POLICY_T;


typedef struct _AUD_VOLUME_POLICY_INFO_
{
    AUD_VOL_POLICY_T eType;		
    AUD_DEC_VOLUME_GAIN_INFO_T rVolGainInfo;	
}AUD_VOLUME_POLICY_INFO;

typedef struct _AUD_DEC_REAR_VOLME_GAIN_INFO_T
{
    __u32    u4RearVolGain;
}AUD_DEC_REAR_VOLUME_GAIN_INFO_T;

typedef enum
{
    AUD_DEC_TESTTONE_FRONT,
    AUD_DEC_TESTTONE_REAR    
}AUD_DEC_TESTTONE_OUT;

typedef enum
{
    AUD_DEC_TEST_TONE_PINK_NOISE,
    AUD_DEC_TEST_TONE_TRIANGLE_WAVE,
    AUD_DEC_TEST_TONE_SINE_WAVE,
    AUD_DEC_TEST_TONE_WHITE_NOISE,
    AUD_DEC_TEST_TONE_PINK_NOISE_DOLBY  // pink noise with Dolby required level
} AUD_DEC_TEST_TONE_TYPE_T;

typedef struct
{
    AUD_DEC_TEST_TONE_TYPE_T         eTTType;//just set AUD_DEC_LS_SPK_ALL
    AUD_DEC_TESTTONE_OUT eTTOut;
}AUD_TESTTONE_SET_TYPE;

typedef enum
{
    AUD_DEC_TESTTONE_ENABLE = 0,
    AUD_DEC_TESTTONE_DISABLE
}AUD_DEC_TESTTONE_ONOFF;

typedef struct
{
    AUD_DEC_TESTTONE_ONOFF         eTTSwitch;//just set AUD_DEC_LS_SPK_ALL
    AUD_DEC_TESTTONE_OUT eTTOut;
}AUD_TESTTONE_SWITCH_T;

typedef struct
{
    AUD_DEC_LS_T         eTTLs;//just set AUD_DEC_LS_SPK_ALL
    AUD_DEC_TESTTONE_OUT eTTOut;
}AUD_TESTTONE_SET;

typedef enum{
    AUD_DAC_PWM,
    AUD_DAC_EXT,
}AUD_DAC_TYPE_T;

typedef struct _AUD_DAC_TYPE_SEL_T
{
    AUD_CFG_ID eOut;
    AUD_DAC_TYPE_T eDacType;
}AUD_DAC_TYPE_SEL_T;

typedef struct
{
    __u8 u1_total_spk_num;
    __u64 u8_spk_layout;

    __u16 u2_front_size;  //Front Speaker Size, value: 0/5/10/15/20/.../290/295/300.
    __u16 u2_center_size; //Center Speaker Size,  value: 0/5/10/15/20/.../290/295/300.
    __u16 u2_rear_size;   //Rear Speaker Size,  value: 0/5/10/15/20/.../290/295/300.
    __u16 u2_sub_size;    //Sub Speaker Size,  value: 0/5/10/15/20/.../290/295/300.
    __u32 u4_sub_force_out;
}AUD_DEC_SPEAKER_LAYOUT_T;

typedef struct pcm_volume {
	__s32 u4LVolume;
	__s32 u4RVolume;
	__s32 policy;
} PCM_VOLUME;

typedef struct AUD_SE_MVS_COEF_T
{
    __u32        u4GainScale;            //Fixed 3
    __u32        u4WidthGain;
    __u32        u4LRGain;
    __u32        u4CenterGain;
    __u32        u4CrosstalkGain;
    __u32        u4BassGain;
    __u32        u4OutputGain;
    __u32        u4InputGain;
    __u32        u4VRPhase;                //0(default)/1
}AUD_SE_MVS_COEF_T;

typedef enum
{
    AUD_SE_ATS_SWITCH_OFF = 0,
    AUD_SE_ATS_SWITCH_ON
}   AUD_SE_ATS_SWITCH_T;

typedef struct AUD_SE_ATS_CTRL_INFO_TAG
{
    AUD_SE_ATS_CTRL_T    e_ctrlID; /* IN */
    union {
        AUD_SE_ATS_SWITCH_T e_ats_switch;
        __u32 u4CtrlMode;   
        __u32 u4InputGain;                         
        __u32 u4CenterGain;
        __u32 u4LRGain;
        __u32 u4LsRsGain;
        __u32 u4LfeGain;
        __u32 u4CenterInGain;
        __u32 u4LfeInGain;
        __u32 u4LsRsInGain;    
        __u32 u4C2LRGain;
        __u32 u4C2LsRsGain;
        __u32 u4Lr2LsRsGain;  
        __u32 u4OverallDelay;
    } u;
} AUD_SE_ATS_CTRL_INFO_T;

int  OpenAudioDev()
{
    int fd = open(AUD_DEV_NAME, O_RDWR);

    if (fd < 0)
    {
        printf("OpenAudioDev Failed!\r\n");
    }
    
    return (fd);
}

int  OpenPCMDev()
{
    int fd = open(PCM_DEV, O_RDWR);

    if (fd < 0)
    {
        printf("OpenPCMDev Failed!\r\n");
    }
    
    return (fd);
}


int  CloseAudioDev(int fd)
{
    return close(fd);
}

int SetMute(AUD_MUTE_TYPE_T  eMute)
{
    AUD_DEC_MUTE_TYPE_T  eAudioMute;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetMute OpenAudioDev fail! \r\n");
        return ret;    
    }
    
    if(AUD_MUTE_ON == eMute)
    {
        eAudioMute = AUD_DEC_MUTE_ON;
    }
    else
    {
        eAudioMute = AUD_DEC_MUTE_OFF;
    }

    pdata.pInBuf = &eAudioMute;
    pdata.InSize = sizeof(AUD_DEC_MUTE_TYPE_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_MUTE_TYPE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int SetMicMute(bool fgMute)
{
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;
    __u32 u4MicMute = 0;
    
    if (fgMute == true)
    {
        u4MicMute = 1;
    }
    else
    {
        u4MicMute = 0;
    }

    printf("[GSYS]SetMicMute: u4MicMute=%d\r\n", u4MicMute);

    if((fd = OpenPCMDev()) < 0)
    {
        printf("SetMicMute OpenPCMDev fail \r\n");
        return ret;    
    }

    pdata.pInBuf = &u4MicMute;
    pdata.InSize = sizeof(u4MicMute);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, SET_MIC_MUTE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}


__u32  SetVolumeEx(__s32 u4Vol)
{ 
    AUD_DEC_VOLUME_INFO_T     sAudioVolume;
    sAudioVolume.e_vol_type = AUD_DEC_ALL_CH;
    sAudioVolume.u.ui1_level = (__u8)u4Vol;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetVolumeEx OpenAudioDev fail \r\n");
        return ret;    
    }

    pdata.pInBuf = &sAudioVolume;
    pdata.InSize = sizeof(AUD_DEC_VOLUME_INFO_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_VOLUME, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int SetVolume(int u4Vol)
{
    
    int volumeGain[41] =
    {
        0, 0x5be, 0x672, 0x73b, 0x81d, 0x91a, 0xa37, 0xb76, 0xcdc, 0xe6e,
        0x1030, 0x122a, 0x1462, 0x16de, 0x19a9, 0x1cca, 0x204e, 0x243f, 0x28ab, 0x2da1,
        0x3333, 0x3972, 0x4074, 0x4852, 0x5125, 0x5b0c, 0x6628, 0x729f, 0x809b, 0x904d,
        0xa1e8, 0xb5aa, 0xcbd4, 0xe4b3, 0x1009b, 0x11feb, 0x1430c, 0x16a77, 0x196b2, 0x1c852, 0x20000
    };

    int index = -1;

    for(index = 0; index < 41; index ++)
    {
        if(u4Vol == volumeGain[index])
        {
            break;
        }
    }
    if(index > 40)
    {
        return -1;
    }

    return SetStreamTypeVolume(AUD_STREAM_TYPE_MM,index);


#if 0

    int fd = -1, ret = -1;
    AUD_VOLUME_POLICY_INFO sAudioVolume;
    WIN32_IOCTL_DATA pdata;

    if ((u4Vol < 0))
    {
        printf("SetVolume param u4Vol is must larger 0!\r\n");
        return ret;
    }

    fd = open(AUD_DEV_NAME, O_RDWR);
    if (fd < 0)
    {
        printf("SetVolume: OpenAudioDev error!\r\n");
        return ret;
    }

    sAudioVolume.eType = AUD_VOL_NORMAL;
    sAudioVolume.rVolGainInfo.e_vol_type = AUD_DEC_ALL_CH;
    sAudioVolume.rVolGainInfo.u.u4FrontMasterVolGain = u4Vol;

    pdata.pInBuf = &sAudioVolume;
    pdata.InSize = sizeof(AUD_DEC_VOLUME_GAIN_INFO_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_VOL_POLICY, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
#endif
}

__u32 SetRearVolumeEx(__u32 u4Vol)
{
    AUD_DEC_REAR_VOLUME_INFO_T     sAudioRearVolume;
    sAudioRearVolume.ui1_level = (__u8)u4Vol;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetRearVolumeEx OpenAudioDev fail\r\n");
        return -1;    
    }

    pdata.pInBuf = &sAudioRearVolume;
    pdata.InSize = sizeof(AUD_DEC_REAR_VOLUME_INFO_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUD_SET_REAR_VOLUME, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}


int  SetRearVolume(__u32 u4Vol)
{
    AUD_DEC_REAR_VOLUME_INFO_T     sAudioRearVolume;
    sAudioRearVolume.ui1_level = u4Vol;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetRearVolume OpenAudioDev fail \r\n");
        return ret;    
    }

    pdata.pInBuf = &sAudioRearVolume;
    pdata.InSize = sizeof(AUD_DEC_REAR_VOLUME_INFO_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUD_SET_REAR_VOLUME, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int GetEQType(EQTYPE_T *peEQType)
{
    if(NULL == peEQType)
    {
        printf("[GSYS]GGetEQType parameter peEQType is NULL! \r\n");
        return -1;
    }

    *peEQType = (EQTYPE_T)DEFAULT_VALUE_EQTYPE;
   
    return 0;
}


int SetEQValues(AUD_EQVALUES_T rEQValues)
{
    AUD_SE_OPCMD_T rOpCmd;
    AUD_DEC_EQ_TYPE_V2_T audEQParm = {0};
    __u8     uIndex = 0, uIndex2 = 0;
    __u32 szGainValues[EQBAND_NUM] = {0};
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetEQValues OpenAudioDev fail! \r\n");
        return ret;    
    }

    memcpy(szGainValues, &rEQValues, sizeof(szGainValues));

    audEQParm.u4_enable = 0;
    for (uIndex = 1; uIndex < 11; uIndex++)
    {
        if (0 != szGainValues[uIndex])
        {
            audEQParm.u4_enable = 1;
            break;
        }
    }

    audEQParm.u4_bandnum = 9;
    for (uIndex = 0; uIndex < 7; uIndex++)
    {
        for (uIndex2 = 0; uIndex2 < 11; uIndex2++)
        {
            audEQParm.au4_gain[uIndex][uIndex2] = szGainValues[uIndex2];
        }
    }
    

    rOpCmd.u1Type = AUD_SE_EQUALIZER;
    rOpCmd.pvData = &audEQParm;
    rOpCmd.u4DataSize = sizeof(AUD_DEC_EQ_TYPE_V2_T);
    rOpCmd.u4OpCode = AUD_DEC_EQ_SET_U4ENABLE | AUD_DEC_EQ_SET_U4BANDNUM | AUD_DEC_EQ_SET_AU4GAIN;

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int GetEQValues(AUD_EQVALUES_T* prEQValues)
{
    AUD_EQVALUES_T rDfltEQValues = {DEFAULT_VALUE_EQBAND0, DEFAULT_VALUE_EQBAND1,
                                 DEFAULT_VALUE_EQBAND2, DEFAULT_VALUE_EQBAND3,
                                 DEFAULT_VALUE_EQBAND4, DEFAULT_VALUE_EQBAND5,
                                 DEFAULT_VALUE_EQBAND6, DEFAULT_VALUE_EQBAND7,
                                 DEFAULT_VALUE_EQBAND8, DEFAULT_VALUE_EQBAND9,
                                 DEFAULT_VALUE_EQBAND10};

    if(NULL == prEQValues)
    {
        printf("[GSYS]GGetEQValues parameter prEQValues is NULL! \r\n");
        return -1;
    }

    *prEQValues = rDfltEQValues;
    
    return 0;    
}

int SetEQType(EQTYPE_T eEQType, MISC_EQ_GAIN_T rEQTypeGain)
{
    AUD_SE_OPCMD_T rOpCmd;
    AUD_DEC_EQ_TYPE_V2_T audEQParm;
    __u8     uIndex = 0, uIndex2 = 0;
    
    __u32    szGainValues[EQBAND_NUM] = {0};
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetEQType OpenAudioDev fail! \r\n");
        return ret;    
    }
    
    szGainValues[0] = rEQTypeGain.u4Gain0;
    szGainValues[1] = rEQTypeGain.u4Gain1;
    szGainValues[2] = rEQTypeGain.u4Gain2;
    szGainValues[3] = rEQTypeGain.u4Gain3;
    szGainValues[4] = rEQTypeGain.u4Gain4;
    szGainValues[5] = rEQTypeGain.u4Gain5;
    szGainValues[6] = rEQTypeGain.u4Gain6;
    szGainValues[7] = rEQTypeGain.u4Gain7;
    szGainValues[8] = rEQTypeGain.u4Gain8;
    szGainValues[9] = rEQTypeGain.u4Gain9;
    szGainValues[10] = rEQTypeGain.u4Gain10;

    audEQParm.u4_enable = 0;        
    for(uIndex = 1; uIndex < 11; uIndex++)
    {
        if(0 != szGainValues[uIndex])
        {
            audEQParm.u4_enable = 1;
            break;
        }
    }        
    
    audEQParm.u4_bandnum = 9;
    for(uIndex = 0; uIndex < 7; uIndex++)
    {
        for(uIndex2 = 0; uIndex2 < 11; uIndex2++)
        {
            audEQParm.au4_gain[uIndex][uIndex2] = szGainValues[uIndex2];
        }       
    }    
    
    rOpCmd.u1Type = AUD_SE_EQUALIZER;
    rOpCmd.pvData = &audEQParm;
    rOpCmd.u4DataSize = sizeof(AUD_DEC_EQ_TYPE_V2_T);
    rOpCmd.u4OpCode = AUD_DEC_EQ_SET_U4ENABLE | AUD_DEC_EQ_SET_U4BANDNUM | AUD_DEC_EQ_SET_AU4GAIN;

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int SetTestTone(int u4TestTone, int u4TestToneType)
{
    AUD_OUTPUT_PATH_T rOutputPath;
    AUD_TESTTONE_SET rTTSetCfg;
    AUD_TESTTONE_SET_TYPE rTestToneType;
    AUD_TESTTONE_SWITCH_T rTestToneSwitch;
    int fd = -1, u4Result = -1;
    WIN32_IOCTL_DATA pdata;

    printf("SetTestTone: u4TestTone=%d, u4TestToneType=%d\r\n",
        u4TestTone, u4TestToneType);
    if ((u4TestTone < TEST_TONE_FRONT) || (u4TestTone > TEST_TONE_OFF))
    {
        printf("SetTestTone: u4TestTone error!\r\n");
        return u4Result;
    }
    if ((u4TestToneType < MISC_TEST_TONE_PINK_NOISE) || (u4TestToneType > MISC_TEST_TONE_PINK_NOISE_DOLBY))
    {
        printf( "SetTestTone: TestTone Type error!\r\n");
        return u4Result;
    }

    switch (u4TestTone)
    {
    case TEST_TONE_FRONT:
        rOutputPath.eOut = AUD_FRONT;
        rOutputPath.eSrc = AUD_AOUT1;
        rTestToneType.eTTType = (AUD_DEC_TEST_TONE_TYPE_T)u4TestToneType;
        rTestToneType.eTTOut = AUD_DEC_TESTTONE_FRONT;
        rTestToneSwitch.eTTSwitch = AUD_DEC_TESTTONE_ENABLE;
        rTestToneSwitch.eTTOut = AUD_DEC_TESTTONE_FRONT;
        rTTSetCfg.eTTLs = AUD_DEC_LS_SPK_ALL;
        rTTSetCfg.eTTOut = AUD_DEC_TESTTONE_FRONT;
        break;

    case TEST_TONE_REAR:
        rOutputPath.eOut = AUD_REAR;
        rOutputPath.eSrc = AUD_AOUT2;
        rTestToneType.eTTType = (AUD_DEC_TEST_TONE_TYPE_T)u4TestToneType;
        rTestToneType.eTTOut = AUD_DEC_TESTTONE_REAR;
        rTestToneSwitch.eTTSwitch = AUD_DEC_TESTTONE_ENABLE;
        rTestToneSwitch.eTTOut = AUD_DEC_TESTTONE_REAR;
        rTTSetCfg.eTTLs = AUD_DEC_LS_SPK_ALL;
        rTTSetCfg.eTTOut  = AUD_DEC_TESTTONE_REAR;
        break;

    case TEST_TONE_OFF:
        rTestToneSwitch.eTTSwitch = AUD_DEC_TESTTONE_DISABLE;
        rTestToneSwitch.eTTOut = AUD_DEC_TESTTONE_REAR;
        break;
    }
    
    if ((fd = OpenAudioDev()) < 0)
    {
        printf( "[GSYS]SetTestTone: OpenAudioDev error!\r\n");
        return u4Result;
    }
    if (u4TestTone != TEST_TONE_OFF)
    {
        pdata.pInBuf = &rOutputPath;
        pdata.InSize = sizeof(AUD_OUTPUT_PATH_T);
        pdata.pOutBuf = NULL;
        pdata.OutSize = 0;
        pdata.pBytesReturned = NULL;

        u4Result = ioctl(fd, IOCTL_AUDIO_AOUT_CONFIG, &pdata);
        
        if (u4Result < 0)
        {
            printf( "[GSYS]SetTestTone: IOCTL_AUDIO_AOUT_CONFIG error!\r\n");
            close(fd);
            fd  = -1;
            return (u4Result);
        }

        pdata.pInBuf = &rTestToneType;
        pdata.InSize = sizeof(AUD_TESTTONE_SET_TYPE);
        pdata.pOutBuf = NULL;
        pdata.OutSize = 0;
        pdata.pBytesReturned = NULL;

        u4Result = ioctl(fd, IOCTL_AUD_SET_TEST_TONE_TYPE, &pdata);
        if (u4Result < 0)
        {
            printf( "[GSYS]SetTestTone: IOCTL_AUDIO_SET_TEST_TONE_TYPE error!\r\n");
            close(fd);
            fd  = -1;
            return (u4Result);
        }

        pdata.pInBuf = &rTestToneSwitch;
        pdata.InSize = sizeof(AUD_TESTTONE_SWITCH_T);
        pdata.pOutBuf = NULL;
        pdata.OutSize = 0;
        pdata.pBytesReturned = NULL;

        u4Result = ioctl(fd, IOCTL_AUD_SET_TEST_TONE_ONOFF, &pdata);
        if (u4Result < 0)
        {
            printf( "[GSYS]SetTestTone: IOCTL_AUDIO_SET_TEST_TONE_ONOFF error!\r\n");
            close(fd);
            fd  = -1;
            return (u4Result);
        }

        pdata.pInBuf = &rTTSetCfg;
        pdata.InSize = sizeof(AUD_TESTTONE_SET);
        pdata.pOutBuf = NULL;
        pdata.OutSize = 0;
        pdata.pBytesReturned = NULL;

        u4Result = ioctl(fd, IOCTL_AUD_SET_TEST_TONE_CHANNEL, &pdata);
        if (u4Result < 0)
        {
            printf( "[GSYS]SetTestTone: IOCTL_AUDIO_SET_TEST_TONE_CHANNEL error!\r\n");
            close(fd);
            fd  = -1;
            return (u4Result);
        }
    }
    else
    {
        pdata.pInBuf = &rTestToneSwitch;
        pdata.InSize = sizeof(AUD_TESTTONE_SWITCH_T);
        pdata.pOutBuf = NULL;
        pdata.OutSize = 0;
        pdata.pBytesReturned = NULL;

        u4Result = ioctl(fd, IOCTL_AUD_SET_TEST_TONE_ONOFF, &pdata);
        if (u4Result < 0)
        {
            printf( "[GSYS]SetTestTone: IOCTL_AUDIO_SET_TEST_TONE_ONOFF error!\r\n");
            close(fd);
            fd  = -1;
            return (u4Result);
        }
    }

    close(fd);
    fd = -1;
    
    return 0;
}

int SetBalance(__u32 u4Values, BALANCE_CHANGE_TYPE eBalanceType)
{
    AUD_DEC_VOLUME_GAIN_INFO_T sAudioVolume;
    sAudioVolume.e_vol_type = AUD_DEC_INDIVIDUAL_CH;
    sAudioVolume.u.t_ch_gain_vol.u4FrontChVolGain = u4Values;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetBalance OpenAudioDev fail\r\n");
        return ret;    
    }

    printf("SetBalance i4Values is 0x%x, eBalanceType is %d\r\n", u4Values, eBalanceType);
    
    switch(eBalanceType)
    {
        case BALANCE_FRONT_LEFT:
            sAudioVolume.u.t_ch_gain_vol.e_ls = AUD_DEC_LS_FRONT_LEFT;
            break;
            
        case BALANCE_FRONT_RIGHT:
            sAudioVolume.u.t_ch_gain_vol.e_ls = AUD_DEC_LS_FRONT_RIGHT;
            break;
            
        case BALANCE_REAR_LEFT:
            sAudioVolume.u.t_ch_gain_vol.e_ls = AUD_DEC_LS_REAR_LEFT;
            break;
            
        case BALANCE_REAR_RIGHT:
            sAudioVolume.u.t_ch_gain_vol.e_ls = AUD_DEC_LS_REAR_RIGHT;
            break;
            
        case BALANCE_CENTER:
            sAudioVolume.u.t_ch_gain_vol.e_ls = AUD_DEC_LS_CENTER;
            break;
            
        case BALANCE_SUB_WOOFER:
            sAudioVolume.u.t_ch_gain_vol.e_ls = AUD_DEC_LS_SUB_WOOFER;
            break;
            
        default:
            return ret;
    }

    pdata.pInBuf = &sAudioVolume;
    pdata.InSize = sizeof(AUD_DEC_VOLUME_GAIN_INFO_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_VOLUME, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}


int SetSpeakerLayout(AUD_SPEAKER_LAYOUT_T rSpkSz)
{
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;
    AUD_DEC_SPEAKER_LAYOUT_T rSpkLayout;
    
    memset((void*)&rSpkLayout, 0, sizeof(AUD_DEC_SPEAKER_LAYOUT_T));

    fd = open(AUD_DEV_NAME, O_RDWR);

    if (fd < 0)
    {
        return ret;
    }

    switch(rSpkSz.u4SpeakerLayoutType & SPEAKER_LAYOUT_MAX)
    {
    case SPEAKER_LAYOUT_LR:
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_LR | SPK_LAYOUT_LR_EXIST);
        break;

    case SPEAKER_LAYOUT_MONO:
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_LR | SPK_LAYOUT_LR_EXIST);
        break;

    case SPEAKER_LAYOUT_STEREO:
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_STEREO); //
        break;

    case SPEAKER_LAYOUT_LRC:
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_LRC | SPK_LAYOUT_C_EXIST | SPK_LAYOUT_LR_EXIST);
        break;

    case SPEAKER_LAYOUT_LRS:
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_LRS | SPK_LAYOUT_LR_EXIST);//how to do s
        break;

    case SPEAKER_LAYOUT_LRCS:
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_LRCS | SPK_LAYOUT_LR_EXIST | SPK_LAYOUT_C_EXIST);//how to do s
        break;

    case SPEAKER_LAYOUT_LRLSRS:
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_LRLSRS | SPK_LAYOUT_LR_EXIST | SPK_LAYOUT_LSRS_EXIST);
        break;

    case SPEAKER_LAYOUT_LRCLSRS:
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_LRCLSRS | SPK_LAYOUT_LR_EXIST | SPK_LAYOUT_C_EXIST | SPK_LAYOUT_LSRS_EXIST);
        break;

    default:
        printf("[GSYS][SDK_SERVICE]SetSpeakerLayout: Spk layout unknown!\r\n");
        break;
    }

    if (rSpkSz.u4SpeakerLayoutType & SPEAKER_LAYOUT_SUBWOOFER)
    {
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_SUBWOOFER | SPK_LAYOUT_LFE_EXIST);
    }
    
    //Spk size
    if (0 == rSpkSz.u2CenterSize)
    {
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_C_LARGE);
    }

    if(0 == rSpkSz.u2FrontSize)
    {
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_L_LARGE);        
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_R_LARGE);
    } 

    if (0 == rSpkSz.u2RearSize)
    {
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_LS_LARGE);
        rSpkLayout.u8_spk_layout |= (SPK_LAYOUT_RS_LARGE);
    }   

    rSpkLayout.u2_front_size = rSpkSz.u2FrontSize;
    rSpkLayout.u2_center_size = rSpkSz.u2CenterSize;
    rSpkLayout.u2_rear_size = rSpkSz.u2RearSize;
    rSpkLayout.u2_sub_size = rSpkSz.u2SubSize;
    rSpkLayout.u4_sub_force_out = rSpkSz.u4SubForceOut;

    pdata.pInBuf = &rSpkLayout;
    pdata.InSize = sizeof(AUD_DEC_SPEAKER_LAYOUT_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SPEAKER_LAYOUT, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int SetUpMix(AUD_UPMIX_T eUpMixType, MISC_UPMIX_GAIN_T rUpmixGain)
{
    AUD_SE_OPCMD_T    rOpCmd;
    AUD_SE_UPMIX_CTRL_INFO_T    audUpmix;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetUpMix OpenAudioDev fail \r\n");
        return ret;    
    }
    
    switch(eUpMixType) 
    {
        case AUD_UPMIX_ON:            
            rOpCmd.u4OpCode = AUD_SE_OPCODE_ON | AUD_SE_OPCODE_UPG_COEF;
            audUpmix.e_upmix_mode = AUD_SE_UPMIX_MODE_ON;
            break;
            
        case AUD_UPMIX_OFF:
            rOpCmd.u4OpCode = AUD_SE_OPCODE_OFF;
            audUpmix.e_upmix_mode = AUD_SE_UPMIX_MODE_OFF;
            break;            
        
        default:
            return ret;
    }
    
    audUpmix.UPMIX_GAIN[0] = rUpmixGain.u4Gain0;
    audUpmix.UPMIX_GAIN[1] = rUpmixGain.u4Gain1;
    audUpmix.UPMIX_GAIN[2] = rUpmixGain.u4Gain2;
    audUpmix.UPMIX_GAIN[3] = rUpmixGain.u4Gain3;
    audUpmix.UPMIX_GAIN[4] = rUpmixGain.u4Gain4;
    audUpmix.UPMIX_GAIN[5] = rUpmixGain.u4Gain5;
    audUpmix.UPMIX_GAIN[6] = rUpmixGain.u4Gain6;
    audUpmix.UPMIX_GAIN[7] = rUpmixGain.u4Gain7;
    
    rOpCmd.u1Type = AUD_SE_UPMIX;
    rOpCmd.pvData = &audUpmix;
    rOpCmd.u4DataSize = sizeof(AUD_SE_UPMIX_CTRL_INFO_T);

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret; 
}

int SetLoudNess(__u8 uLoudNessType, MISC_LOUDNESS_GAIN_T rLoudNessGain)
{    
    AUD_SE_OPCMD_T    rOpCmd;
    AUD_SE_LOUDNESS_CTRL_INFO_T        audLoudness;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetEQType OpenAudioDev fail! \r\n");
        return ret;    
    }
    
    if(uLoudNessType > 20)
    {
        printf("[GSYS]GClientSetLoudNess parameter uLoudNessType is error! \r\n");
        return ret;
    }
    
    audLoudness.e_loudness_mode = (AUD_SE_LOUDNESS_MODE_T)uLoudNessType;    
    audLoudness.Loud_GAIN[0] = rLoudNessGain.u4Gain0;
    audLoudness.Loud_GAIN[1] = rLoudNessGain.u4Gain1;
    audLoudness.Loud_GAIN[2] = rLoudNessGain.u4Gain2;
    audLoudness.Loud_GAIN[3] = rLoudNessGain.u4Gain3;
    audLoudness.Loud_GAIN[4] = rLoudNessGain.u4Gain4;
    audLoudness.Loud_GAIN[5] = rLoudNessGain.u4Gain5;            
    
    rOpCmd.u1Type = AUD_SE_LOUDNESS;
    rOpCmd.pvData = &audLoudness;
    rOpCmd.u4DataSize = sizeof(AUD_SE_LOUDNESS_CTRL_INFO_T);
    if (uLoudNessType == AUD_SE_LOUDNESS_0dB)
    {
        rOpCmd.u4OpCode = AUD_SE_OPCODE_OFF | AUD_SE_OPCODE_UPG_COEF;
    }
    else
    {
        rOpCmd.u4OpCode = AUD_SE_OPCODE_ON | AUD_SE_OPCODE_UPG_COEF;
    }
           
    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int SetSpdifOutputType(SPDIFOUTPUT_T eSpdifOutputType)
{
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;
    
    if ((fd = OpenAudioDev()) < 0)
    {
        printf("SetSpdifOutputType OpenAudioDev fail\r\n");
        return ret;
    }

    pdata.pInBuf = &eSpdifOutputType;
    pdata.InSize = sizeof(eSpdifOutputType);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUD_SET_TYPE_SPDIF, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int ChooseSpdifOutput(MISC_AUD_OUT_TYPE_T eOutType)
{
    AUD_OUT_TYPE_T eAudOut = {0};
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("ChooseSpdifOutput OpenAudioDev fail\r\n");
        return ret;    
    }
    
    switch(eOutType)
    {
        case MISC_AUD_AOUT1:
            eAudOut = AUD_AOUT1;
            break;
        case MISC_AUD_DVD_OUT:
            eAudOut = AUD_DVD_OUT;
            break;
        case MISC_AUD_OFF:
            eAudOut = AUD_UNDEF_OUT;
            break;
        default :
            return ret;
    }

    pdata.pInBuf = &eAudOut;
    pdata.InSize = sizeof(AUD_OUT_TYPE_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUD_SPDIF_ENABLE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int SetAudFeature(AUD_DEC_FEATURE_INFO_T eAudFeatur)
{
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;
    
    if ((fd = OpenAudioDev()) < 0)
    {
        return ret;
    }

    pdata.pInBuf = &eAudFeatur;
    pdata.InSize = sizeof(AUD_DEC_FEATURE_INFO_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_FEATURE, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int AudioFuncOptionSet(MISC_AUD_FUNC_OPTION_T rFuncOptionSet)
{
    AUD_FUNC_OPTION_T    rAudFuncOption = {0}; 
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;
    
    if((fd = OpenAudioDev()) < 0)
    {
        printf("AudioFuncOptionSet  OpenAudioDev fail\r\n");
        return ret;    
    }
    
    memcpy(&rAudFuncOption, &rFuncOptionSet, sizeof(AUD_FUNC_OPTION_T));

    pdata.pInBuf = &rAudFuncOption;
    pdata.InSize = sizeof(AUD_FUNC_OPTION_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUD_FUNC_OPTION_SET, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;       
}

int SetAudThreshold(MISC_AUD_THRESHOLD_T rThreshold)
{
    AUD_THRESHOLD_T rAudThreshold = {0};
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if ((fd = OpenAudioDev()) < 0) {
        printf("SetAudThreshold OpenAudioDev fail\r\n");
        return ret; 
    }
    
    memcpy(&rAudThreshold, &rThreshold, sizeof(AUD_THRESHOLD_T));

    pdata.pInBuf = &rAudThreshold;
    pdata.InSize = sizeof(AUD_THRESHOLD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUD_SET_THRESHOLD, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;        
}


int AudioSetDiversityInfo(AUD_DEC_DIV_INFO_T *prAudDivInfo)
{
    AUD_DEC_DIV_INFO_T rAudDivInfo;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;
    
    if ((fd = OpenAudioDev()) < 0 || NULL == prAudDivInfo)
    {
        return ret;
    }
    if (AUD_DEC_DIV_TYPE_CODEC_SUPPORT < prAudDivInfo->e_type)
    {
        return ret;
    }
    
    rAudDivInfo.e_type = prAudDivInfo->e_type;
    rAudDivInfo.u1_setting = prAudDivInfo->u1_setting;

    pdata.pInBuf = &rAudDivInfo;
    pdata.InSize = sizeof(rAudDivInfo);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_DIVERSITY_INFO, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}


int SetPL2(PLII_TYPE ePL2Type)
{
    AUD_SE_OPCMD_T rOpCmd;
    AUD_SE_PL2_CTRL_INFO_T rPL2Cmd;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if ((fd = OpenAudioDev()) < 0)
    {
        printf( "SetPL2: OpenAudioDev error!\r\n");
        return ret;
    }
    printf( "SetPL2: ePL2Type=%d\r\n", ePL2Type);

    switch (ePL2Type)
    {
    case AUD_SE_PL2_OFF:
        rPL2Cmd.e_ctrlID = AUD_SE_PL2_CTRL_SWITCH;
        rPL2Cmd.u_value.e_pl2_switch = AUD_SE_PL2_SWITCH_OFF;
        break;

    case AUD_SE_PL2_MOVIE:
        rPL2Cmd.e_ctrlID = AUD_SE_PL2_CTRL_MODE;
        rPL2Cmd.u_value.e_pl2_mode = AUD_SE_PL2_MODE_MOVIE;
        break;

    case AUD_SE_PL2_MUSIC:
        rPL2Cmd.e_ctrlID = AUD_SE_PL2_CTRL_MODE;
        rPL2Cmd.u_value.e_pl2_mode = AUD_SE_PL2_MODE_MUSIC;
        break;

    case AUD_SE_PL2_AUTO:
        rPL2Cmd.e_ctrlID = AUD_SE_PL2_CTRL_SWITCH;
        rPL2Cmd.u_value.e_pl2_switch = AUD_SE_PL2_SWITCH_ON;
        break;

    default:
        rPL2Cmd.e_ctrlID = AUD_SE_PL2_CTRL_SWITCH;
        rPL2Cmd.u_value.e_pl2_switch = AUD_SE_PL2_SWITCH_OFF;
        break;
    }

    rOpCmd.u1Type = AUD_SE_PROLOGICII;
    rOpCmd.pvData = &rPL2Cmd;
    rOpCmd.u4DataSize = sizeof(AUD_SE_PL2_CTRL_INFO_T);
    rOpCmd.u4OpCode = AUD_SE_OPCODE_CTRL;

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    SetAudFeature(AUD_DEC_PROLOGICII);
    
    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int SetSRSSwitch(MISC_AUD_SE_CSII_SWITCH_T eCSIISwitch)
{
    AUD_SE_OPCMD_T rOpCmd = {0};
    AUD_SE_CSII_CTRL_INFO_T rCsiiCtrlInfo;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetSRSSwitch OpenAudioDev fail\r\n");
        return ret;    
    }
    
    rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_SWITCH;        

    switch(eCSIISwitch)
    {
        case MISC_AUD_SE_CSII_OFF:
            rCsiiCtrlInfo.u.e_csii_switch = AUD_SE_CSII_SWITCH_OFF;
            break;
            
        case MISC_AUD_SE_CSII_ON:
            rCsiiCtrlInfo.u.e_csii_switch = AUD_SE_CSII_SWITCH_ON;
            break;
            
        case MISC_AUD_SE_CSII_SWITCH_AUTO:
            rCsiiCtrlInfo.u.e_csii_switch = AUD_SE_CSII_SWITCH_AUTO;
            break;
            
        default:
            return ret;
    }

    rOpCmd.u1Type = AUD_SE_CSII;
    rOpCmd.u4OpCode = AUD_SE_OPCODE_CTRL;
    rOpCmd.pvData   = &rCsiiCtrlInfo;
    rOpCmd.u4DataSize = sizeof(AUD_SE_CSII_CTRL_INFO_T);

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
        
}

int SetSRSMode(MISC_AUD_SE_CSII_MODE_T eCSIIMode)
{
    AUD_SE_OPCMD_T rOpCmd = {0};
    AUD_SE_CSII_CTRL_INFO_T rCsiiCtrlInfo;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;
    
    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetSRSMode OpenAudioDev fail\r\n");
        return ret;    
    }
    
    rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_MODE;

    switch(eCSIIMode)
    {
        case MISC_AUD_SE_CSII_MODE_CINEMA:
            rCsiiCtrlInfo.u.e_mode = AUD_SE_CSII_MODE_CINEMA;
            break;
            
        case MISC_AUD_SE_CSII_MODE_PRO:
            rCsiiCtrlInfo.u.e_mode = AUD_SE_CSII_MODE_PRO;
            break;
            
        case MISC_AUD_SE_CSII_MODE_MUSIC:
            rCsiiCtrlInfo.u.e_mode = AUD_SE_CSII_MODE_MUSIC;
            break;
            
        case MISC_AUD_SE_CSII_MODE_MONO:
            rCsiiCtrlInfo.u.e_mode = AUD_SE_CSII_MODE_MONO;
            break;
            
        case MISC_AUD_SE_CSII_MODE_LCRS:
            rCsiiCtrlInfo.u.e_mode = AUD_SE_CSII_MODE_LCRS;
            break;
            
        default:
            return ret;
    }

    rOpCmd.u1Type = AUD_SE_CSII;
    rOpCmd.u4OpCode = AUD_SE_OPCODE_CTRL;
    rOpCmd.pvData   = &rCsiiCtrlInfo;
    rOpCmd.u4DataSize = sizeof(AUD_SE_CSII_CTRL_INFO_T);

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);

    if (ret >= 0)
    {
        return ret;
    }

    close(fd);
    fd = -1;

    return ret;
}

__u32 GetSRSMode(MISC_AUD_SE_CSII_MODE_T *peCSIIMode)
{
    if(NULL == peCSIIMode)
    {
        printf("GetSRSMode parameter peCSIIMode is NULL! \r\n");
        return -1;
    }

    *peCSIIMode = (MISC_AUD_SE_CSII_MODE_T)DEFAULT_VALUE_SRSMODE;   
    return 0;
}

int SetSRSPhantom(MISC_AUD_SE_CSII_SWITCH_T eCSIISwitch)
{
    AUD_SE_OPCMD_T rOpCmd = {0};
    AUD_SE_CSII_CTRL_INFO_T rCsiiCtrlInfo;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetSRSPhantom OpenAudioDev fail\r\n");
        return ret;    
    }
    
    rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_PHANTOM;

    switch(eCSIISwitch)
    {
        case MISC_AUD_SE_CSII_OFF:
            rCsiiCtrlInfo.u.e_phantom = AUD_SE_CSII_PHANTOM_OFF;
            break;
            
        case MISC_AUD_SE_CSII_ON:
            rCsiiCtrlInfo.u.e_phantom = AUD_SE_CSII_PHANTOM_ON;
            break;
            
        default:
            return ret;
    }

    rOpCmd.u1Type = AUD_SE_CSII;
    rOpCmd.u4OpCode = AUD_SE_OPCODE_CTRL;
    rOpCmd.pvData   = &rCsiiCtrlInfo;
    rOpCmd.u4DataSize = sizeof(AUD_SE_CSII_CTRL_INFO_T);

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int GetSRSPhantom(MISC_AUD_SE_CSII_SWITCH_T *peCSIISwitch)
{
    if(NULL == peCSIISwitch)
    {
        printf("GetSRSPhantom parameter peCSIISwitch is NULL! \r\n");
        return -1;
    }

    *peCSIISwitch = (MISC_AUD_SE_CSII_SWITCH_T)DEFAULT_VALUE_SRSPHANTOM;    
    
    return 0;
}

int SetSRSFullBand(MISC_AUD_SE_CSII_SWITCH_T eCSIISwitch)
{
    AUD_SE_OPCMD_T rOpCmd = {0};
    AUD_SE_CSII_CTRL_INFO_T rCsiiCtrlInfo;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetSRSFullBand OpenAudioDev fail\r\n");
        return ret;    
    }
    
    rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_FB;

    switch(eCSIISwitch)
    {
        case MISC_AUD_SE_CSII_OFF:
            rCsiiCtrlInfo.u.e_fb = AUD_SE_CSII_FB_OFF;
            break;
            
        case MISC_AUD_SE_CSII_ON:
            rCsiiCtrlInfo.u.e_fb = AUD_SE_CSII_FB_ON;
            break;
            
        default:
            return ret;
    }

    rOpCmd.u1Type = AUD_SE_CSII;
    rOpCmd.u4OpCode = AUD_SE_OPCODE_CTRL;
    rOpCmd.pvData   = &rCsiiCtrlInfo;
    rOpCmd.u4DataSize = sizeof(AUD_SE_CSII_CTRL_INFO_T);

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;
    
    return ret;
}

__u32 GetSRSFullBand(MISC_AUD_SE_CSII_SWITCH_T *peCSIISwitch)
{

    if(NULL == peCSIISwitch)
    {
        printf("GetSRSFullBand parameter peCSIISwitch is NULL! \r\n");
        return -1;
    }

    *peCSIISwitch = (MISC_AUD_SE_CSII_SWITCH_T)DEFAULT_VALUE_SRSFULLBAND;
    
    return 0;
}

int SetSRSFocus(MISC_AUD_SE_CSII_FOCUS_T eFocus, MISC_AUD_SE_CSII_SWITCH_T eCSIISwitch)
{
    AUD_SE_OPCMD_T rOpCmd = {0};
    AUD_SE_CSII_CTRL_INFO_T rCsiiCtrlInfo;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetSRSFocus OpenAudioDev fail\r\n");
        return ret;    
    }
    
    switch(eFocus)
    {
        case MISC_AUD_SE_CSII_FOCUS_CENTER:
            rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_FOCUS_CENTER;
            if(eCSIISwitch == MISC_AUD_SE_CSII_OFF)
            {
                rCsiiCtrlInfo.u.e_focuscenter = AUD_SE_CSII_FOCUSC_OFF;
            }
            else
            {
                rCsiiCtrlInfo.u.e_focuscenter = AUD_SE_CSII_FOCUSC_ON;
            }
            break; 
            
        case MISC_AUD_SE_CSII_FOCUS_FRONT:
            rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_FOCUS_FRONT;
            if(eCSIISwitch == MISC_AUD_SE_CSII_OFF)
            {
                rCsiiCtrlInfo.u.e_focusfront = AUD_SE_CSII_FOCUSF_OFF;
            }
            else
            {
                rCsiiCtrlInfo.u.e_focusfront = AUD_SE_CSII_FOCUSF_ON;
            }
            break;
            
        case MISC_AUD_SE_CSII_FOCUS_REAR:
            rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_FOCUS_REAR;
            if(eCSIISwitch == MISC_AUD_SE_CSII_OFF)
            {
                rCsiiCtrlInfo.u.e_focusrear = AUD_SE_CSII_FOCUSR_OFF;
            }
            else
            {
                rCsiiCtrlInfo.u.e_focusrear = AUD_SE_CSII_FOCUSR_ON;
            }
            break;
            
        default:
            return ret;
    }
    
    rOpCmd.u1Type = AUD_SE_CSII;
    rOpCmd.u4OpCode = AUD_SE_OPCODE_CTRL;
    rOpCmd.pvData   = &rCsiiCtrlInfo;
    rOpCmd.u4DataSize = sizeof(AUD_SE_CSII_CTRL_INFO_T);

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;
   
    return ret;
}

int GetSRSFocus(MISC_AUD_SE_CSII_FOCUS_T eFocus, MISC_AUD_SE_CSII_SWITCH_T *peCSIISwitch)
{

    if(NULL == peCSIISwitch)
    {
        printf("[GSYS]GGetSRSFocus parameter peCSIISwitch is NULL! \r\n");
        return -1;
    }
    
    switch(eFocus)
    {
        case MISC_AUD_SE_CSII_FOCUS_CENTER:
            *peCSIISwitch = (MISC_AUD_SE_CSII_SWITCH_T)DEFAULT_VALUE_FOCUS0;
            break; 
            
        case MISC_AUD_SE_CSII_FOCUS_FRONT:
            *peCSIISwitch = (MISC_AUD_SE_CSII_SWITCH_T)DEFAULT_VALUE_FOCUS1;
            break;
            
        case MISC_AUD_SE_CSII_FOCUS_REAR:
            *peCSIISwitch = (MISC_AUD_SE_CSII_SWITCH_T)DEFAULT_VALUE_FOCUS2;
            break;
            
        default:
            return -1;
    }
            
    
    return 0;
}

int SetSRSTrueBass(MISC_AUD_SE_CSII_SWITCH_T eCSIISwitch)
{
    AUD_SE_OPCMD_T rOpCmd = {0};
    AUD_SE_CSII_CTRL_INFO_T rCsiiCtrlInfo;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetSRSTrueBass OpenAudioDev fail! \r\n");
        return ret;    
    }
    
    rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_TB;
    
    switch(eCSIISwitch)
    {
        case MISC_AUD_SE_CSII_OFF:
            rCsiiCtrlInfo.u.e_TB = AUD_SE_CSII_TB_OFF;
            break;
            
        case MISC_AUD_SE_CSII_ON:
            rCsiiCtrlInfo.u.e_TB = AUD_SE_CSII_TB_ON;
            break;
            
        default:
            return ret;
    }

    rOpCmd.u1Type = AUD_SE_CSII;
    rOpCmd.u4OpCode = AUD_SE_OPCODE_CTRL;
    rOpCmd.pvData   = &rCsiiCtrlInfo;
    rOpCmd.u4DataSize = sizeof(AUD_SE_CSII_CTRL_INFO_T);

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);

    if (ret >= 0)
    {
        return ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int GetSRSTrueBass(MISC_AUD_SE_CSII_SWITCH_T *peCSIISwitch)
{
    

    if(NULL == peCSIISwitch)
    {
        printf("[GSYS]GGetSRSTrueBass parameter peCSIISwitch is NULL! \r\n");
        return -1;
    }

    *peCSIISwitch = (MISC_AUD_SE_CSII_SWITCH_T)DEFAULT_VALUE_SRSTB;
    
    return 0;
}

int SetSRSTrueBassSize(MISC_AUD_SE_CSII_TBSS_T eTBSS, MISC_AUD_SE_CSII_SS_T eCSIITBSS)
{
    AUD_SE_OPCMD_T rOpCmd = {0};
    AUD_SE_CSII_CTRL_INFO_T rCsiiCtrlInfo;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetSRSTrueBassSize OpenAudioDev fail\r\n");
        return ret;    
    }    

    switch(eCSIITBSS)
    {
        case MISC_AUD_SE_CSII_SS_40HZ:
            if(MISC_AUD_SE_CSII_TBSS_FRONT == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_F_SS;
                rCsiiCtrlInfo.u.e_front_ss = AUD_SE_CSII_SS_40HZ;
            }    
            else if(MISC_AUD_SE_CSII_TBSS_SUB == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_S_SS;
                rCsiiCtrlInfo.u.e_sub_ss = AUD_SE_CSII_SS_40HZ;
            }
            else
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_R_SS;
                rCsiiCtrlInfo.u.e_rear_ss = AUD_SE_CSII_SS_40HZ;
            }
            break;  
            
        case MISC_AUD_SE_CSII_SS_60HZ:
            if(MISC_AUD_SE_CSII_TBSS_FRONT == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_F_SS;
                rCsiiCtrlInfo.u.e_front_ss = AUD_SE_CSII_SS_60HZ;
            }    
            else if(MISC_AUD_SE_CSII_TBSS_SUB == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_S_SS;
                rCsiiCtrlInfo.u.e_sub_ss = AUD_SE_CSII_SS_60HZ;
            }            
            else
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_R_SS;
                rCsiiCtrlInfo.u.e_rear_ss = AUD_SE_CSII_SS_60HZ;
            }
            break; 
            
        case MISC_AUD_SE_CSII_SS_100HZ:
            if(MISC_AUD_SE_CSII_TBSS_FRONT == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_F_SS;
                rCsiiCtrlInfo.u.e_front_ss = AUD_SE_CSII_SS_100HZ;
            }    
            else if(MISC_AUD_SE_CSII_TBSS_SUB == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_S_SS;
                rCsiiCtrlInfo.u.e_sub_ss = AUD_SE_CSII_SS_100HZ;
            }
            else
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_R_SS;
                rCsiiCtrlInfo.u.e_rear_ss = AUD_SE_CSII_SS_100HZ;
            }
            break; 
            
        case MISC_AUD_SE_CSII_SS_150HZ:
            if(MISC_AUD_SE_CSII_TBSS_FRONT == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_F_SS;
                rCsiiCtrlInfo.u.e_front_ss = AUD_SE_CSII_SS_150HZ;
            }    
            else if(MISC_AUD_SE_CSII_TBSS_SUB == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_S_SS;
                rCsiiCtrlInfo.u.e_sub_ss = AUD_SE_CSII_SS_150HZ;
            }
            else
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_R_SS;
                rCsiiCtrlInfo.u.e_rear_ss = AUD_SE_CSII_SS_150HZ;
            }
            break;
            
        case MISC_AUD_SE_CSII_SS_200HZ:
            if(MISC_AUD_SE_CSII_TBSS_FRONT == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_F_SS;
                rCsiiCtrlInfo.u.e_front_ss = AUD_SE_CSII_SS_200HZ;
            }    
            else if(MISC_AUD_SE_CSII_TBSS_SUB == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_S_SS;
                rCsiiCtrlInfo.u.e_sub_ss = AUD_SE_CSII_SS_200HZ;
            }            
            else
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_R_SS;
                rCsiiCtrlInfo.u.e_rear_ss = AUD_SE_CSII_SS_200HZ;
            }
            break;            
            
        case MISC_AUD_SE_CSII_SS_250HZ:
            if(MISC_AUD_SE_CSII_TBSS_FRONT == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_F_SS;
                rCsiiCtrlInfo.u.e_front_ss = AUD_SE_CSII_SS_250HZ;
            }    
            else if(MISC_AUD_SE_CSII_TBSS_SUB == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_S_SS;
                rCsiiCtrlInfo.u.e_sub_ss = AUD_SE_CSII_SS_250HZ;
            }            
            else
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_R_SS;
                rCsiiCtrlInfo.u.e_rear_ss = AUD_SE_CSII_SS_250HZ;
            }
            break;
            
        case MISC_AUD_SE_CSII_SS_300HZ:
            if(MISC_AUD_SE_CSII_TBSS_FRONT == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_F_SS;
                rCsiiCtrlInfo.u.e_front_ss = AUD_SE_CSII_SS_300HZ;
            }    
            else if(MISC_AUD_SE_CSII_TBSS_SUB == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_S_SS;
                rCsiiCtrlInfo.u.e_sub_ss = AUD_SE_CSII_SS_300HZ;
            }            
            else
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_R_SS;
                rCsiiCtrlInfo.u.e_rear_ss = AUD_SE_CSII_SS_300HZ;
            }
            break; 
            
        case MISC_AUD_SE_CSII_SS_400HZ:
            if(MISC_AUD_SE_CSII_TBSS_FRONT == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_F_SS;
                rCsiiCtrlInfo.u.e_front_ss = AUD_SE_CSII_SS_400HZ;
            }    
            else if(MISC_AUD_SE_CSII_TBSS_SUB == eTBSS)
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_S_SS;
                rCsiiCtrlInfo.u.e_sub_ss = AUD_SE_CSII_SS_400HZ;
            }
            else
            {
                rCsiiCtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_R_SS;
                rCsiiCtrlInfo.u.e_rear_ss = AUD_SE_CSII_SS_400HZ;
            }
            break;
            
        default:
            return ret;
    }
    
    rOpCmd.u1Type = AUD_SE_CSII;
    rOpCmd.u4OpCode = AUD_SE_OPCODE_CTRL;
    rOpCmd.pvData   = &rCsiiCtrlInfo;
    rOpCmd.u4DataSize = sizeof(AUD_SE_CSII_CTRL_INFO_T);

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);

    if (ret >= 0)
    {
        ret = 0;;
    }

    close(fd);
    fd = -1;
    
    return ret;
}

int GetSRSTrueBassSize(MISC_AUD_SE_CSII_TBSS_T eTBSS, MISC_AUD_SE_CSII_SS_T *peCSIITBSS)
{
    int ret = -1;
    
    if(NULL == peCSIITBSS)
    {
        printf("GetSRSTrueBassSize OpenAudioDev fail\r\n");
        return ret;
    }

    switch(eTBSS)
    {
        case MISC_AUD_SE_CSII_TBSS_FRONT:
            *peCSIITBSS = (MISC_AUD_SE_CSII_SS_T)DEFAULT_VALUE_TBSS0;
            break;  
            
        case MISC_AUD_SE_CSII_TBSS_SUB:
            *peCSIITBSS = (MISC_AUD_SE_CSII_SS_T)DEFAULT_VALUE_TBSS1;
            break; 
            
        case MISC_AUD_SE_CSII_TBSS_REAR:
            *peCSIITBSS = (MISC_AUD_SE_CSII_SS_T)DEFAULT_VALUE_TBSS2;
            break;              
            
        default:
            return ret;
    }
    
    return 0;
}

int SetSRSTrueBassStatus(MISC_AUD_SE_CSII_TBSS_T eTBSS, MISC_AUD_SE_CSII_SWITCH_T eCSIISwitch)
{
    AUD_SE_OPCMD_T rOpCmd = {0};
    AUD_SE_CSII_CTRL_INFO_T rCSIICtrlInfo = {0};
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetRearVolume OpenAudioDev fail \r\n");
        return ret;    
    }

    if ((eTBSS < 0) || (eTBSS > 3))
    {
        return ret;
    }

    switch (eTBSS)
    {
    case MISC_AUD_SE_CSII_TBSS_FRONT:
        rCSIICtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_TBF;
        break;

    case MISC_AUD_SE_CSII_TBSS_SUB:
        rCSIICtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_TBS;
        break;

    case MISC_AUD_SE_CSII_TBSS_REAR:
        rCSIICtrlInfo.e_ctrlID = AUD_SE_CSII_CTRL_TBR;
        break;

    default:
        break;
    }

    if (MISC_AUD_SE_CSII_OFF == eCSIISwitch)
    {
        rCSIICtrlInfo.u.e_TB = AUD_SE_CSII_TB_OFF;
    }
    else
    {
        rCSIICtrlInfo.u.e_TB = AUD_SE_CSII_TB_ON;
    }

    rOpCmd.u1Type = AUD_SE_CSII;
    rOpCmd.pvData = &rCSIICtrlInfo;
    rOpCmd.u4OpCode = AUD_SE_OPCODE_CTRL;
    rOpCmd.u4DataSize = sizeof(AUD_SE_CSII_CTRL_INFO_T);

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;
    
    return ret;
}

int SetSRSLevelValue(MISC_AUD_SE_CSII_SPKID_T eSpkId, __s32 i4LevelValue)
{
    AUD_SE_OPCMD_T rOpCmd = {0};
    AUD_SE_CSII_LEVEL_CTRL_INFO_T rCSIICtrlLevelInfo = {0};
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetRearVolume OpenAudioDev fail \r\n");
        return ret;    
    }

    if ((eSpkId < 16) || (eSpkId > 23))
    {
        return ret;
    }

    rCSIICtrlLevelInfo.e_LevelctrlID = eSpkId;  
    rCSIICtrlLevelInfo.u.i4FocusCenterLevel = i4LevelValue;

    rOpCmd.u1Type = AUD_SE_CSII;
    rOpCmd.pvData = &rCSIICtrlLevelInfo;
    rOpCmd.u4OpCode = AUD_SE_CSII_SET_LEVEL;
    rOpCmd.u4DataSize = sizeof(AUD_SE_CSII_LEVEL_CTRL_INFO_T);

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0; 
    }

    close(fd);
    fd = -1;
    
    return ret;
}

__s32 SetReverbType(REVERBTYPE_T eReverbType, MISC_REVERB_COEF_T rReverbCoef)
{
    AUD_SE_OPCMD_T rOpCmd;
    __s32     i4Idx = -1;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetReverbType OpenAudioDev fail \r\n");
        return ret;    
    }
    
    switch(eReverbType)
    {
        case CFG_REVERB_OFF:
            i4Idx = 6;
        break;
            
        case CFG_REVERB_LIVINGROOM:
            i4Idx = 0;
        break;
            
        case CFG_REVERB_HALL:
            i4Idx = 1;
        break;
            
        case CFG_REVERB_CONCERT:
            i4Idx = 2;
        break;
            
        case CFG_REVERB_CAVE:
            i4Idx = 3;
        break;
            
        case CFG_REVERB_BATHROOM:
            i4Idx = 4;
        break;
            
        case CFG_REVERB_ARENA:
            i4Idx = 5;
        break;

        default:
            i4Idx = -1;
        break;
    }
    
    rOpCmd.u1Type = AUD_SE_REVERB;
    if (i4Idx >= 0 && i4Idx <= 5)
    {
        rOpCmd.pvData = (void *)&rReverbCoef;
        rOpCmd.u4DataSize = sizeof(AUD_SE_REVERB_COEF_T);
        rOpCmd.u4OpCode = AUD_SE_OPCODE_ON | AUD_SE_OPCODE_UPG_COEF;
    }
    else if(i4Idx == 6)
    {
        rOpCmd.pvData = NULL;//(GVOID *)&_arAudSeReverbCoefTab[u4Idx];;
        rOpCmd.u4DataSize = 0;//sizeof(AUD_SE_REVERB_COEF_T);
        rOpCmd.u4OpCode = AUD_SE_OPCODE_OFF;// | AUD_SE_OPCODE_UPG_COEF;
    }

    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;
    
    return ret;
}

__s32 SetBTHFPVolume(__u32 u4Vol)
{
    PCM_VOLUME rBTHFPVol = {0};
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    /*abandon this api*/
    return -1;
    if ((u4Vol < 0) || (u4Vol > 255))
    {
        printf("SetBTHFPVolume: param u4Vol error!\r\n");
        return ret;
    }

    if((fd = OpenPCMDev()) < 0)
    {
        printf("SetBTHFPVolume OpenPCMDev fail \r\n");
        return ret;    
    }

    rBTHFPVol.u4LVolume = u4Vol;
    rBTHFPVol.u4RVolume = u4Vol;
    rBTHFPVol.policy = 0;

    pdata.pInBuf = &rBTHFPVol;
    pdata.InSize = sizeof(rBTHFPVol);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, SET_BT_SPH_GAIN, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;
    
    return ret;
}

__s32 SetPCMDevVolume(__s32 u4LVol, __s32 u4RVol)
{
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;
    PCM_VOLUME rDevVol = {0};

    if ((u4LVol < 0) || (u4LVol > 255) || (u4RVol < 0) || (u4RVol > 255))
    {
        printf("SetPCMDevVolume: param u4LVol or u4RVol error!\r\n");
        return ret;
    }

    if((fd = OpenPCMDev()) < 0)
    {
        printf("SetPCMDevVolume OpenPCMDev fail \r\n");
        return ret;    
    }

    rDevVol.u4LVolume = u4LVol;
    rDevVol.u4RVolume = u4RVol;

    pdata.pInBuf = &rDevVol;
    pdata.InSize = sizeof(rDevVol);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, SET_DEVICE_SPH_GAIN, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;
    
    return ret;
    
}

__s32 SetMVS(MISC_MVS_T eMVSType, MISC_MVS_GAIN_T rMVSGain)
{
    AUD_SE_OPCMD_T    rOpCmd;
    AUD_SE_MVS_COEF_T rAudMVS;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SetUpMix OpenAudioDev fail \r\n");
        return ret;    
    }

    switch(eMVSType) 
    {
        case CFG_MVS_OFF:            
            rOpCmd.u4OpCode = AUD_SE_OPCODE_OFF;
            break;
            
        case CFG_MVS_MODE_MOVIE:
            rOpCmd.u4OpCode = AUD_SE_OPCODE_ON | AUD_SE_OPCODE_UPG_COEF;
            break;    

        case CFG_MVS_MODE_MUSIC:
            rOpCmd.u4OpCode = AUD_SE_OPCODE_ON | AUD_SE_OPCODE_UPG_COEF;
            break;    
        
        default:
            return ret;
    }

    memcpy(&rAudMVS, &rMVSGain, sizeof(AUD_SE_MVS_COEF_T));
    rOpCmd.u1Type = AUD_SE_MVS;
    rOpCmd.pvData = &rAudMVS;
    rOpCmd.u4DataSize = sizeof(AUD_SE_MVS_COEF_T);


    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}


int SelectDAC(int u4Output, int u4Type)
{
    AUD_DAC_TYPE_SEL_T rDACSel;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;
    

    if((fd = OpenAudioDev()) < 0)
    {
        printf("SelectDAC OpenAudioDev fail \r\n");
        return ret;    
    }    
    
    rDACSel.eDacType = u4Type;
    rDACSel.eOut = u4Output;

    if (u4Output == 0)
    {
        if (u4Type == 0)
        {
            printf("[GSYS]SelectDAC: Front Output Select PWM DAC.\r\n");
        }
        else
        {
            printf("[GSYS]SelectDAC: Front Output Select External DAC.\r\n");
        }
    }
    else
    {
        if (u4Type == 0)
        {
            printf("[GSYS]SelectDAC: Rear Output Select PWM DAC.\r\n");
        }
        else
        {
            printf("[GSYS]SelectDAC: Rear Output Select External DAC.\r\n");
        }
    }
    
    pdata.pInBuf = &rDACSel;
    pdata.InSize = sizeof(AUD_DAC_TYPE_SEL_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUD_SET_DAC_TYPE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}


int SetDspMixCh(int u4MixCh)
{
    __u32 u4DspMixCh = 0;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if (u4MixCh > 0x7F)
    {
        printf("[GSYS]SetDspMixCh: param error with u4MixCh > 0x7F\r\n");
        return -1;    
    }
    
    u4DspMixCh = u4MixCh;
    
    if((fd = OpenPCMDev()) < 0)
    {
        printf("SetDspMixCh OpenPCMDev fail \r\n");
        return ret;    
    }
    
    printf("[GSYS]SetDspMixCh: u4MixCh=0x%x\r\n", u4MixCh);

    pdata.pInBuf = &u4DspMixCh;
    pdata.InSize = sizeof(u4DspMixCh);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, SET_DSP_MIX_CH, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int SetPCMConfig(PCM_CUST_CONF* rPCMConfig)
{
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenPCMDev()) < 0)
    {
        printf("SetPCMConfig OpenPCMDev fail \r\n");
        return ret;    
    }
     
    pdata.pInBuf = (void *)&(rPCMConfig->u4SphDelay);
    pdata.InSize = sizeof(rPCMConfig->u4SphDelay);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, SET_SPH_DELAY, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }
    else
    {
        goto exit;
    }

    pdata.pInBuf = (void *)&(rPCMConfig->u4SphMicGain);
    pdata.InSize = sizeof(rPCMConfig->u4SphMicGain);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, SET_SPH_MIC_GAIN, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

exit:
    close(fd);
    fd = -1;

    return ret;
}

int GetPCMConfig(PCM_CUST_CONF* rPCMConfig)
{
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenPCMDev()) < 0)
    {
        printf("GetPCMConfig OpenPCMDev fail \r\n");
        return ret;    
    }
     
    pdata.pInBuf = NULL;
    pdata.InSize = 0;
    pdata.pOutBuf = (void *)&(rPCMConfig->u4SphDelay);
    pdata.OutSize = sizeof(rPCMConfig->u4SphDelay);
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, GET_SPH_DELAY, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }
    else
    {
        goto exit;
    }

    pdata.pInBuf = NULL;
    pdata.InSize = 0;
    pdata.pOutBuf = (void *)&(rPCMConfig->u4SphMicGain);
    pdata.OutSize = sizeof(rPCMConfig->u4SphMicGain);
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, GET_SPH_MIC_GAIN, &pdata);

    if (ret >= 0)
    {
        ret = 0;
    }

exit:
    close(fd);
    fd = -1;

    return ret;
}

int SetPrimaryMic(int u4PrimaryIdx)
{
    __u32 u4PrimaryMicIdx = u4PrimaryIdx;
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if (u4PrimaryIdx > 2)
    {
        printf("[GSYS]SetPrimaryMic: param u4PrimaryIdx(%d) error!\r\n", u4PrimaryIdx);
        return -1;
    }
    
    printf("[GSYS]SetPrimaryMic: u4PrimaryMicIdx = %d\r\n", u4PrimaryMicIdx);
    
    if((fd = OpenPCMDev()) < 0)
    {
        printf("SetPrimaryMic OpenPCMDev fail \r\n");
        return ret;    
    }

    pdata.pInBuf = &u4PrimaryMicIdx;
    pdata.InSize = sizeof(u4PrimaryMicIdx);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, SET_PRIMARY_MIC, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

#if 0 //IOCTL not have
int SetAudRearAoutMode(int u4Mode)
{
	__u32 u4AoutM = u4Mode;
    
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((fd = OpenAudioDev()) < 0)
    {
        printf("%s OpenAudioDev fail \r\n", __FUNCTION__);
        return ret;    
    }    
    
    pdata.pInBuf = &u4AoutM;
    pdata.InSize = sizeof(__U32);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUD_SET_REAR_OUT_MODE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}
#endif

int SetAtsGain(AUD_SE_ATS_CTRL_T eCtrl, int i4Val)
{
    AUD_SE_ATS_CTRL_INFO_T rCtrlInfo;
    AUD_SE_OPCMD_T rOpCmd = {0};
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;

    if((eCtrl < 0)||(eCtrl > AUD_SE_ATS_SURROUND_DELAY))
    {
        printf("[GSYS]SetAtsGain: id error!\r\n");
        return -1;
    }

    rCtrlInfo.e_ctrlID = eCtrl;

    switch (eCtrl)
    {
    case AUD_SE_ATS_CTRL_SWITCH:
        if (0 == i4Val)
        {
            rCtrlInfo.u.e_ats_switch = AUD_SE_ATS_SWITCH_OFF;
        }
        else
        {
            rCtrlInfo.u.e_ats_switch = AUD_SE_ATS_SWITCH_ON;
        }        
        break;

    case AUD_SE_ATS_CTRL_MODE:
        rCtrlInfo.u.u4CtrlMode = i4Val | 1;        
        break;
		
    default:
        rCtrlInfo.u.u4InputGain  = i4Val;
    }
	
    rOpCmd.u1Type = AUD_SE_ATS;
    rOpCmd.pvData = &rCtrlInfo;
    rOpCmd.u4DataSize = sizeof(AUD_SE_ATS_COEF_T);
    rOpCmd.u4OpCode = AUD_SE_OPCODE_CTRL;
	
    if((fd = OpenAudioDev()) < 0)
    {
        printf("%s OpenAudioDev fail \r\n", __FUNCTION__);
        return ret;    
    } 
        
    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;

}

int SetAtsSwitch(int i4Ctrl, AUD_SE_ATS_COEF_T rCoef)
{
    AUD_SE_OPCMD_T rOpCmd = {0};
    int fd = -1, ret = -1;
    WIN32_IOCTL_DATA pdata;
	
    rOpCmd.u1Type = AUD_SE_ATS;
    if(0 == i4Ctrl)
    {
        rOpCmd.pvData = NULL;
        rOpCmd.u4DataSize = 0;
        rOpCmd.u4OpCode = AUD_SE_OPCODE_OFF;
    }
    else
    {
        rOpCmd.pvData = &rCoef;
        rOpCmd.u4DataSize = sizeof(AUD_SE_ATS_COEF_T);
        rOpCmd.u4OpCode = AUD_SE_OPCODE_ON | AUD_SE_OPCODE_UPG_COEF;
    }
	
	
    if((fd = OpenAudioDev()) < 0)
    {
        printf("%s OpenAudioDev fail \r\n", __FUNCTION__);
        return ret;    
    } 
        
    pdata.pInBuf = &rOpCmd;
    pdata.InSize = sizeof(AUD_SE_OPCMD_T);
    pdata.pOutBuf = NULL;
    pdata.OutSize = 0;
    pdata.pBytesReturned = NULL;

    ret = ioctl(fd, IOCTL_AUDIO_SET_SE, &pdata);
    
    if (ret >= 0)
    {
        ret = 0;
    }

    close(fd);
    fd = -1;

    return ret;
}

int SetStreamTypeVolume(int streamType, int index)
{
    return send_stream_type_vol(streamType, index);
}

int GetStreamTypeVolume(int streamType)
{
    return receiveStreamTypeVol(streamType);
}
