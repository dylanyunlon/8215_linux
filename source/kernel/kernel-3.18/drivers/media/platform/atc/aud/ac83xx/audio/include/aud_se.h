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

#ifndef _AUD_SE_H_
#define _AUD_SE_H_

//#include "x_typedef.h"
#include <linux/types.h>
#include <media/atc/drv_aud.h>
#include "chip_ver.h"
#include "aud_drv_config.h"


/*
 *  Audio post process operation code define reference (bit field format)
 */
#define AUD_SE_OPCODE_OFF           (1<<0)  // General Off bit 0
#define AUD_SE_OPCODE_ON            (1<<1)  // General On bit 1
#define AUD_SE_OPCODE_UPG_COEF      (1<<2)  // General Upgrade all coefficient
#define AUD_SE_OPCODE_RESOLVED3     (1<<3)  // General Resolved bit 3
#define AUD_SE_OPCODE_CTRL          (1<<4)  // General Ctrl bit 4
#define AUD_SE_OPCODE_UPG_COEF_HDMI (1<<5)  // General Upgrade coefficient on HDMI
#define AUD_SE_OPCODE_GET_ST        (1<<6)  // Get current status
#define AUD_SE_OPCODE_SET_LEVEL     (1<<7)  // General Set Level bit 7

/*
 *  AUD SE architecture version 2
 */
#if CONFIG_AUD_SE_V2_EN

/*
 *  Post Process Enable Define
 */
#define CONFIG_AUD_SE_XXX_EN            (0)     // (aud_se_xxx) demo used AUD_SE_NOTIFY_NULL
#define CONFIG_AUD_SE_REVERB_EN         (1)
#define CONFIG_AUD_SE_UPMIX_EN          (1)
#define CONFIG_AUD_SE_EQUALIZER_EN      (1)
#define CONFIG_AUD_SE_MP3BOOSTER_EN     (0)
#define CONFIG_AUD_SE_DYNAMIC_BASS_EN   (0)
#define CONFIG_AUD_SE_NEO6_EN           (0)
#define CONFIG_AUD_SE_POSTMIX_EN        (0)     //(aud_se_postmix_v2)
#define CONFIG_AUD_SE_RICH_BASS_EN      (0)     // (aud_se_rbass)
#define CONFIG_AUD_SE_PROLOGICII_EN     (1)     //(aud_se_prologicii_v2)
#define CONFIG_AUD_SE_LOUDNESS_EN       (1)
#define CONFIG_AUD_SE_CSII_EN           (0)     //(aud_se_csii)
#define CONFIG_AUD_SE_SWAP_EN           (0)     //(aud_se_swap_v2)
#define CONFIG_AUD_SE_TVS_EN            (0)     //(aud_se_tvs_v2)
#define CONFIG_AUD_SE_DH2DVS2_EN        (0)     //(aud_se_dh2_dvs2)
#define CONFIG_AUD_SE_PARTY_EN          (0)     //(aud_se_party_v2)
#define CONFIG_AUD_SE_LPF_EN            (0)     //(aud_se_LPF_v2)
#define CONFIG_AUD_SE_3PEQ_EN           (0)     //(aud_se_3peq)
#define CONFIG_AUD_SE_5BEQ_EN           (0)     //(aud_se_5beq)
#define CONFIG_AUD_SE_QBS_EN            (0)     //(aud_se_qbs)
#define CONFIG_AUD_SE_MVS_EN            (1)     //(aud_se_mvs)
#define CONFIG_AUD_SE_ATS_EN            (1)     //(aud_se_ats)
#define CONFIG_AUD_SE_DOWN_SAMPLE       (0)


 /*
  *  reverb types
  */

 /* reverb modes. */
 typedef enum
 {
     AUD_DEC_REVERB_OFF = 0,
     AUD_DEC_REVERB_CONCERT,
     AUD_DEC_REVERB_LIVINGROOM,
     AUD_DEC_REVERB_HALL,
     AUD_DEC_REVERB_BATHROOM,
     AUD_DEC_REVERB_CAVE,
     AUD_DEC_REVERB_ARENA,
     AUD_DEC_REVERB_CHURCH
 }   AUD_DEC_REVERB_MODE_T;


 /*
  *  equalizer types
  */

#define AUD_DEC_EQ_SET_U4ENABLE  (1<<0)
#define AUD_DEC_EQ_SET_U4BANDNUM (1<<1)
#define AUD_DEC_EQ_SET_AU4GAIN   (1<<2)
#define AUD_DEC_EQ_SET_IIR_COEF  (1<<3)


typedef struct AUD_DEC_EQ_TYPE_V2_T
{
     u32 u4_enable;
     u32 u4_bandnum;
     u32 au4_gain[7][11]; // 7 channels, each with 1 dry and 10 band
 }  AUD_DEC_EQ_TYPE_V2_T;

 // 2 groups: positive and negative gain, 48k samplerate only.
 typedef struct _tagAUD_DEC_EQ_TYPE_IIR_COEF_T
 {
     u32 u4IirCoef[2][30];
 } AUD_DEC_EQ_TYPE_IIR_COEF_T;

typedef struct AUD_SE_EQ_INST_T {
	 AUD_DEC_EQ_TYPE_V2_T rUISetting;
	 u32 EQ_TABLE[2][30];
 } AUD_SE_EQ_INST_T;


/*
 *	REVERB Coefficient Struct Type define
 */
#define AUD_SE_REVERB_MAX_BANK_SIZE     (79)
#define AUD_SE_REVERB_FS_CNT            (3)         // support 3 sample rate type 32/44/48
#define AUD_SE_REVERB_BANK_CNT          (4)         // reverb bank count 4

typedef struct {
    u32 u4Gain;
    u32 u4FeedBackGain;
    u32 u4BankSize[AUD_SE_REVERB_FS_CNT][AUD_SE_REVERB_BANK_CNT];
} AUD_SE_REVERB_COEF_T;

typedef struct {
    bool fgUIEnableFlag;							// UI enable Flag
    bool fgDspSupportFlag;							// System support Flag
    bool fgCoefFlag;								// Coefficient Setting flag
    AUD_SE_REVERB_COEF_T rCoef; 					// Internal used save related coefficient space
} AUD_SE_REVERB_INST_T;


/*
 *  Prologic II types
 */

typedef enum
{
    AUD_DRV_PL2_CTRL_SWITCH = 0,
    AUD_DRV_PL2_CTRL_MODE,
    AUD_DRV_PL2_CTRL_PANORAMA,
    AUD_DRV_PL2_CTRL_DIMENSION,
    AUD_DRV_PL2_CTRL_C_WIDTH
}   AUD_DRV_PL2_CTRL_T;

typedef enum
{
    AUD_DRV_PL2_SWITCH_ON = 0,
    AUD_DRV_PL2_SWITCH_OFF,
    AUD_DRV_PL2_SWITCH_AUTO
}   AUD_DRV_PL2_SWITCH_MODE_T;

typedef enum
{
    AUD_DRV_PL2_MODE_MODE_PROLGIC_EMULATION = 0,
    AUD_DRV_PL2_MODE_MODE_VIRTUAL,
    AUD_DRV_PL2_MODE_MODE_MUSIC,
    AUD_DRV_PL2_MODE_MODE_MOVIE,
    AUD_DRV_PL2_MODE_MODE_MATRIX,
    AUD_DRV_PL2_MODE_MODE_CUSTOM
}   AUD_DRV_PL2_MODE_MODE_T;

typedef enum
{
    AUD_DRV_PL2_PANORAMA_ON = 0,
    AUD_DRV_PL2_PANORAMA_OFF
}   AUD_DRV_PL2_PANORAMA_T;

typedef enum
{
    AUD_DRV_PL2_DIMENSION_0 = 0,
    AUD_DRV_PL2_DIMENSION_1,
    AUD_DRV_PL2_DIMENSION_2,
    AUD_DRV_PL2_DIMENSION_3,
    AUD_DRV_PL2_DIMENSION_4,
    AUD_DRV_PL2_DIMENSION_5,
    AUD_DRV_PL2_DIMENSION_6
}   AUD_DRV_PL2_DIMENTION_T;

typedef enum
{
    AUD_DRV_PL2_C_WIDTH_0 = 0,
    AUD_DRV_PL2_C_WIDTH_1,
    AUD_DRV_PL2_C_WIDTH_2,
    AUD_DRV_PL2_C_WIDTH_3,
    AUD_DRV_PL2_C_WIDTH_4,
    AUD_DRV_PL2_C_WIDTH_5,
    AUD_DRV_PL2_C_WIDTH_6,
    AUD_DRV_PL2_C_WIDTH_7
}   AUD_DRV_PL2_C_WIDTH_T;


typedef struct _AUD_DRV_PL2_VAL_MIN_MAX_T
{
    u8      ui1_curr_val;
    u8      ui1_min_val; /* Only used in AUD_DRV_GET_TYPE_PL2_CTRL */
    u8      ui1_max_val; /* Only used in AUD_DRV_GET_TYPE_PL2_CTRL */
} AUD_DRV_PL2_VAL_MIN_MAX_T;

typedef struct _AUD_DRV_PL2_CTRL_INFO_T
{
    AUD_DRV_PL2_CTRL_T    e_ctrl; /* IN */

    union
    {
        AUD_DRV_PL2_SWITCH_MODE_T   e_pl2_switch;
        AUD_DRV_PL2_MODE_MODE_T     e_pl2_mode;
        bool                        b_is_pl2_panorama_on;
        AUD_DRV_PL2_VAL_MIN_MAX_T   t_pl2_val;
    } u;

} AUD_DRV_PL2_CTRL_INFO_T;


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

/* Dimension operation value */
typedef enum
{
    AUD_SE_PL2_DIMENSION_0 = 0,
    AUD_SE_PL2_DIMENSION_1,
    AUD_SE_PL2_DIMENSION_2,
    AUD_SE_PL2_DIMENSION_3,
    AUD_SE_PL2_DIMENSION_4,
    AUD_SE_PL2_DIMENSION_5,
    AUD_SE_PL2_DIMENSION_6
}   AUD_SE_PL2_DIMENTION_T;

/* C width operation value */
typedef enum
{
    AUD_SE_PL2_C_WIDTH_0 = 0,
    AUD_SE_PL2_C_WIDTH_1,
    AUD_SE_PL2_C_WIDTH_2,
    AUD_SE_PL2_C_WIDTH_3,
    AUD_SE_PL2_C_WIDTH_4,
    AUD_SE_PL2_C_WIDTH_5,
    AUD_SE_PL2_C_WIDTH_6,
    AUD_SE_PL2_C_WIDTH_7
}   AUD_SE_PL2_C_WIDTH_T;

typedef struct AUD_SE_PL2_VAL_MIN_MAX_T
{
    u8      ui1_curr_val;
    u8      ui1_min_val; /* Only used in AUD_DRV_GET_TYPE_PL2_CTRL */
    u8      ui1_max_val; /* Only used in AUD_DRV_GET_TYPE_PL2_CTRL */
} AUD_SE_PL2_VAL_MIN_MAX_T;

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

typedef struct {
    bool fgUIEnableFlag;
    bool fgDspSupportFlag;
    bool fgCoefFlag;
    AUD_SE_PL2_CTRL_INFO_T rCoef;
} AUD_SE_PL2_INST_T;


typedef enum
{
    AUD_DEC_PL2_CTRL_SWITCH = 0,
    AUD_DEC_PL2_CTRL_MODE,
    AUD_DEC_PL2_CTRL_PANORAMA,
    AUD_DEC_PL2_CTRL_DIMENSION,
    AUD_DEC_PL2_CTRL_C_WIDTH
}   AUD_DEC_PL2_CTRL_T;

typedef enum
{
    AUD_DEC_PL2_SWITCH_ON = 0,
    AUD_DEC_PL2_SWITCH_OFF,
    AUD_DEC_PL2_SWITCH_AUTO
}   AUD_DEC_PL2_SWITCH_MODE_T;

typedef enum
{
    AUD_DEC_PL2_MODE_MODE_PROLGIC_EMULATION = 0,
    AUD_DEC_PL2_MODE_MODE_VIRTUAL,
    AUD_DEC_PL2_MODE_MODE_MUSIC,
    AUD_DEC_PL2_MODE_MODE_MOVIE,
    AUD_DEC_PL2_MODE_MODE_MATRIX,
    AUD_DEC_PL2_MODE_MODE_CUSTOM
}   AUD_DEC_PL2_MODE_MODE_T;

typedef enum
{
    AUD_DEC_PL2_PANORAMA_ON = 0,
    AUD_DEC_PL2_PANORAMA_OFF
}   AUD_DEC_PL2_PANORAMA_T;



/*
 *  Upmix types
 */

/* mode operation value */
typedef enum
{
    AUD_SE_UPMIX_MODE_OFF = 0,
    AUD_SE_UPMIX_MODE_ON
}   AUD_SE_UPMIX_MODE_T;


typedef struct AUD_SE_UPMIX_CTRL_INFO_T
{
    AUD_SE_UPMIX_MODE_T     e_upmix_mode;
    u32                     UPMIX_GAIN[8];
} AUD_SE_UPMIX_CTRL_INFO_T;


typedef struct {
    bool fgUIEnableFlag;                        // UI enable Flag
    bool fgDspSupportFlag;                      // System support Flag
    bool fgCoefFlag;                            // Coefficient Setting flag
    AUD_SE_UPMIX_CTRL_INFO_T rCoef;             // Internal used save related coefficient space
} AUD_SE_UPMIX_INST_T;


/*
 *  loudness types
 */

/* mode operation value */
typedef enum
{
    AUD_SE_LOUDNESS_0dB = 0,
    AUD_SE_LOUDNESS_1dB,
    AUD_SE_LOUDNESS_2dB,
    AUD_SE_LOUDNESS_3dB,
    AUD_SE_LOUDNESS_4dB,
    AUD_SE_LOUDNESS_5dB,
    AUD_SE_LOUDNESS_6dB,
    AUD_SE_LOUDNESS_7dB,
    AUD_SE_LOUDNESS_8dB,
    AUD_SE_LOUDNESS_9dB,
    AUD_SE_LOUDNESS_10dB,
    AUD_SE_LOUDNESS_11dB,
    AUD_SE_LOUDNESS_12dB,
    AUD_SE_LOUDNESS_13dB,
    AUD_SE_LOUDNESS_14dB,
    AUD_SE_LOUDNESS_15dB,
    AUD_SE_LOUDNESS_16dB,
    AUD_SE_LOUDNESS_17dB,
    AUD_SE_LOUDNESS_18dB,
    AUD_SE_LOUDNESS_19dB,
    AUD_SE_LOUDNESS_20dB
}   AUD_SE_LOUDNESS_MODE_T;

typedef struct AUD_SE_LOUDNESS_CTRL_INFO_T
{
  AUD_SE_LOUDNESS_MODE_T       e_loudness_mode;
  u32                          Loud_GAIN[6];
} AUD_SE_LOUDNESS_CTRL_INFO_T;

typedef struct {
    bool fgUIEnableFlag;
    bool fgDspSupportFlag;
    bool fgCoefFlag;
    AUD_SE_LOUDNESS_CTRL_INFO_T rCoef;
} AUD_SE_LOUDNESS_INST_T;


/*
 *  CSII Coefficient Struct Type define
 */



/* switch operation value */
typedef enum
{
    AUD_SE_CSII_SWITCH_OFF = 0,
    AUD_SE_CSII_SWITCH_ON,
    AUD_SE_CSII_SWITCH_AUTO
}   AUD_SE_CSII_SWITCH_T;

/* mode operation value */
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

/* CSII Focus & TrueBass & Mix2Rear Level setting. */
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
}AUD_SE_CSII_SPKID_T;


typedef struct AUD_SE_CSII_LEVEL_CTRL_INFO_TAG
{
    AUD_SE_CSII_SPKID_T    e_LevelctrlID; /* IN */
	union {
        s32 i4FocusCenterLevel;
        s32 i4FocusFrontLevel;
		s32 i4FocusRearLevel;
        s32 i4TBFrontLevel;
		s32 i4TBSubLevel;
		s32 i4TBRearLevel;
		s32 i4Front2RearLevel;
		s32 i4Center2RearLevel;
    } u;
}AUD_SE_CSII_LEVEL_CTRL_INFO_T;


/*--------------ATS---------------------*/

/* switch operation value */
typedef enum
{
    AUD_SE_ATS_MODE_OFF       = 0x0,
    AUD_SE_ATS_MODE_ON        = 0x1,
    AUD_SE_ATS_MONO_MODE      = 0x1<<8,
    AUD_SE_ATS_DO_LSRS_FILTER = 0x1<<16
}AUD_SE_ATS_MODE_T;

typedef struct AUD_SE_ATS_INT_COEF_T
{
    u32 u4LsRsBandNum;
    u32 u4IIROrder;
} AUD_SE_ATS_INT_COEF_T;

typedef struct {
    bool fgUIEnableFlag;                        // UI enable Flag
    bool fgDspSupportFlag;                      // System support Flag
    bool fgCoefFlag;                            // Coefficient Setting flag
    AUD_SE_ATS_COEF_T rCoef;                  // Internal used save related coefficient space
    AUD_SE_ATS_INT_COEF_T rIntCoef;
    u32 uBand0CoefNum;
} AUD_SE_ATS_INST_T;

/*
 *  MVS Coefficient Struct Type define
 */

typedef struct {
    u32 u4GainScale;                         // Fixed 3
    u32 u4WidthGain;
    u32 u4LRGain;
    u32 u4CenterGain;
    u32 u4CrosstalkGain;
    u32 u4BassGain;
    u32 u4OutputGain;
    u32 u4InputGain;
    u32 u4VRPhase;                           // 0(default) / 1
} AUD_SE_MVS_COEF_T;

typedef struct {
    bool fgUIEnableFlag;                        // UI enable Flag
    bool fgDspSupportFlag;                      // System support Flag
    u32 u4Mod;                               // 0->None (default), 1->2ch bypass (aud_se_mvs_mod1) Water Add MVS mod1(auto mod) (bypass stereo source) 2011-10-12
    AUD_SE_MVS_COEF_T rCoef;                    // Internal used save related coefficient space
} AUD_SE_MVS_INST_T;




/*
 *  Audio post process UOP Command define
 */
typedef enum {
    AUD_SE_UOP_NULL = 0,                    // Resolved NULL opcode
                                            // _XXX_ UOP Opcode define
    AUD_SE_UOP__XXX__ON,                    // _XXX_ On UOP Opcode
    AUD_SE_UOP__XXX__OFF,                   // _XXX_ Off UOP Opcode
    // Add new uop opcode to here...
    AUD_SE_UOP_MAX_OPCODE = 0x00FFFF        // Resolved max uop opcode
} AUD_SE_UOP_T;



/*
 *  Audio post process query open used environment information
 */
typedef struct _tagAudSeEnvInfo {
    AUD_SOURCE_CFG_T *prDspSrcParam;        // prDspSrcParam = &g_rDspSrcParam
    AUD_OUTPUT_SETTING_CFG_T *prDspOutputParam; // prDspOutputParam = &g_rDspOutParam
    u32 u4FreqIdx;                       // Current Frequency Index
} AUD_SE_ENV_INFO_T;

/*
 *  Audio post process object struct
 */
typedef struct _tagAudSeObj
{
    void    (*vInit)(void);                 // [Option/NULL] Obj init function
    u32  (*u4ProcessOpCmd)(              // [Option/NULL] Obj Process Operation Command Function
                u32 u4OpCode,            //      [in] Operation Command Code
                void *pvData,               //      [in] Command related information data pointer
                u32 u4DataSize);         //      [in] related information data size
    u32  (*u4ProcessNotify)(             // [Option/NULL] Obj Process Notify function
                u32 u4NotifyType,        //      [in] current notify type
                void *pInfo);               //      [in/out] notify related information data pointer
    u32  (*u4ProcessUOP)(                // [Option/NULL] Obj Process UOP function
                u16 u2UopOpCode);        //      [in] Uop operation command code
    bool    (*fgQueryOpen)(                 // [Must] Obj function on / off query by system or other obj
                AUD_SE_ENV_INFO_T *ptEnv);  //      [in] current environment
    u32  (*u4GetCurStatus)(void);        // [Must] Obj stauts report
    u32  (*u4ProcessCLICmd)(             // [Must] Obj CLI debug interface
                u32 u4CLICmd);           //      [in] CLI Command Type
    AUD_SE_TYPE_T   u1Type;
} AUD_SE_OBJ_T;

/*
 *  Audio post process interface
 */
void vAudSeInit(void);                      // Audio post process init function

bool fgAudSeProcessOpCmd(                   // Audio process Operation command function
        void *pvInfo);                      // Operation command information pointer

void vAudSeProcessNotify(                   // Audio post process notify function
        u32 u4NotifyType,                // Current notify type
        void *pInfo);                       // notify related information data pointer

void vAudSeProcessUOP(                      // Audio process UOP function
        u32 u4Uop);                      // current uop id

void vAudSeSendUop(                         // send post process UOP function base on type and uop opcode
        u8 u1Type,                       // uop target post process type
        u16 u2UopOpCode);                // real uop opcode

const AUD_SE_OBJ_T * prAudSeGetObjPtr(      // get special post process obj pointer
        AUD_SE_TYPE_T u1Type);                    // targe post process obj type

const AUD_SE_OBJ_T * AudSeGetObjPtrBaseInd(u32 u4Index);


void vResetPostReinitFlag(void);
bool fgAudSeProcessCLIOpCmd(void *pvInfo);

#if CONFIG_AUD_SE_EQUALIZER_EN
void vAudSeEQOneBandCLICmd(u32 u4BandNum, u32 u4GainIdx);
void vAudSeEQIIRCoefCLICmd(u32 u4BandNum, u32 u4CoefIdx, u32 u4CoefData);
#endif

#if ((CONFIG_DRV_AUDIO_EXTERNAL_POST_PROC_SUPPORT)||(CONFIG_DRV_AUDIO_EXTERNAL_POST_PROC_ON_ARM1))
void vAudSeExtPPCLICmd(u32 u4Opmod,u32 uPara2,u32 uPara3);
#endif

typedef struct
{
    s8 *pszStr;
    u32 u4Idx;
} AUD_SE_COMMENT_T;




/*
 *  Audio post process status define
 */
typedef enum {
    AUD_SE_ST_OK = 0x0000,                  // General audio post process "OK" Status
    AUD_SE_ST_OK_NO_REINIT,                 // "OK" Status, but dsp not need do reinit
                                            // Process Operation Code related status define
    AUD_SE_ST_UNKNOW_OPCODE = 0x1000,       // Unkown Operation Code for this post process object
                                            // Process Notify related status define
    AUD_SE_ST_NOPROC_NOTIFY = 0x2000,       // This post process object don't process this notify
    AUD_SE_ST_PROERR_NOTIFY,                // This post process object process current notify failed
                                            // Process UOP related status define
    AUD_SE_ST_UNKNOW_UOPCOD = 0x3000,       // Unkown UopOpCode
                                            // Current Status define
    AUD_SE_ST_STATUS_ON     = 0x4000,       // Current post process object status is on
    AUD_SE_ST_STATUS_OFF,                   // Current post process object status is off
                                            // CLI Process status
    AUD_SE_ST_UNKNOW_CLICMD = 0x5000,       // This post process object can't process this CLI command
    AUD_SE_ST_ERR = 0x00FFFFFF              // Undefined "ERR" Status
} AUD_SE_STATUS_T;


/*
 *  Audio post process CLI Command define
 */
typedef enum {
                                            // Generel CLI Command support
    AUD_SE_CLI_ST_GUEST = 1,                // Guest level cli log show current post process status (version, current coef...)
    AUD_SE_CLI_ST_SUPER,                    // Super level cli log show current post process status (add more internal information base on guest level)
                                            // Other Special CLI Command base on real post process object
} AUD_SE_CLI_CMD_T;

typedef struct _tagAudSeCliCmd{
    AUD_SE_TYPE_T  u1Type;                  // target post process type
    AUD_SE_CLI_CMD_T u1CliCmd;
} AUD_SE_CLICMD_T;


/*
 *  Audio post process Notify Type Define
 */
typedef enum {
    AUD_SE_NOTIFY_NULL = 0,                 // Resolved NULL type
    AUD_SE_NOTIFY_LOAD_POSTCODE,            // Load DSP post process related code
    AUD_SE_NOTIFY_BQAVD,                    // Berore Query AVD Notify
    AUD_SE_NOTIFY_ENDAVD,                    //After Query AVD Notify
    AUD_SE_NOTIFY_SET_PP_TAB,               // In vSetPostProcTable Function (after Query AVD)
    AUD_SE_NOTIFY_SET_SPK_CFG,               // In vSetPostProcTable Function (after Query AVD)
    AUD_SE_NOTIFY_SET_PLAY_INIT
} AUD_SE_NOTIFY_T;


typedef struct {                            // AUD_SE_NOTIFY_LOAD_POSTCODE
    const u8  *pu1Type;                  // DSP Support Post process type list pointer
    u32 u4TypeCnt;                       // DSP Support post process type count
} AUD_SE_NOTIFY_LOAD_POSTCODE_T;

typedef struct {                            // AUD_SE_NOTIFY_BQAVD
    AUD_SOURCE_CFG_T *prDspSrcParam;        // prDspSrcParam = &g_rDspSrcParam
    u32 u4FreqIdx;                       // Current Frequency Index
} AUD_SE_NOTIFY_BQAVD_T;

typedef struct {                            // AUD_SE_NOTIFY_AQAVD
    AUD_SOURCE_CFG_T *prDspSrcParam;        // prDspSrcParam = &g_rDspSrcParam
    AUD_OUTPUT_SETTING_CFG_T *prDspOutputParam; // prDspOutputParam = &g_rDspOutParam
    AUD_OUTPUT_SETTING_CFG_T *prDspOutputHdmiParam; // prDspOutputParam = &g_rDspOutHdmiParam
} AUD_SE_NOTIFY_ENDAVD_T;

typedef struct {                            // AUD_SE_NOTIFY_AQAVD
    AUD_SOURCE_CFG_T *prDspSrcParam;        // prDspSrcParam = &g_rDspSrcParam
    AUD_OUTPUT_SETTING_CFG_T *prDspOutputParamHDMI; // prDspOutputParam = &g_rDspOutParam
} AUD_SE_NOTIFY_SET_HDMI_SPK_T;



typedef struct {                            // AUD_SE_NOTIFY_AQAVD
    AUD_SOURCE_CFG_T *prDspSrcParam;        // prDspSrcParam = &g_rDspSrcParam
    AUD_OUTPUT_SETTING_CFG_T *prDspOutputParam; // prDspOutputParam = &g_rDspOutParam
    u32 u4FreqIdx;                       // Current Frequency Index
} AUD_SE_NOTIFY_SET_PP_TAB_T;

/* Query Interface */
bool fgAudSeGetRunStatus(           //Query current runtime status of post process
        AUD_SE_TYPE_T u1Type);                      // targe post process obj type

bool fgAudSeGetPL2Mode(                   //Query current neo6 mode
        AUD_DEC_PL2_MODE_MODE_T* pePL2Mode);     // Current Mode of PL2



#endif  // #if CONFIG_AUD_SE_V2_EN



extern void AudSeSetReinit(void);
extern void AudSeSetPostDramOk(void);

enum AUDIO_SE_DBG_SWITCH_E
{
    DBG_SWITCH_REVERB = 0,
    DBG_SWITCH_EQ,
    DBG_SWITCH_PAE,
    DBG_SWITCH_DBASS,
    DBG_SWITCH_NEO6,
    DBG_SWITCH_POSTMIX,
    DBG_SWITCH_RBASS,
    DBG_SWITCH_PL2,
    DBG_SWITCH_SWAP,
    DBG_SWITCH_TVS,
    DBG_SWITCH_LPF,
    DBG_SWITCH_PEQ,
    DBG_SWITCH_BEQ
};

extern void SetAudSeDebugMode(u32 u4Type, bool bypassFlag);
extern AUD_SE_EQ_INST_T* AudSeGetEqInst(void);
extern AUD_SE_UPMIX_INST_T* AudSeGetUpmixInst(void);
extern AUD_SE_REVERB_INST_T* AudSeGetReverbInst(void);
extern AUD_SE_LOUDNESS_INST_T* AudSeGetLoudnessInst(void);
extern AUD_SE_ATS_INST_T* AudSeGetAtsInst(void);
extern AUD_SE_MVS_INST_T* AudSeGetMvsInst(void);
#endif  // #ifndef _AUD_SE_H_


