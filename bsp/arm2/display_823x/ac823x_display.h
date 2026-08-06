#ifndef __AC823X_DISPLAY_H__
#define __AC823X_DISPLAY_H__

#define PMX_1 0
#define PMX_2 1

#define PMX_HW_VIDEO_LAYER 0  
#define PMX_HW_OSD1_LAYER 4  
#define PMX_HW_OSD2_LAYER 1  
#define PMX_HW_OSD3_LAYER 2  
#define PMX_HW_OSD4_LAYER 3  

#define PMX_HW_PLANE_1    0
#define PMX_HW_PLANE_2    1
#define PMX_HW_PLANE_3    2
#define PMX_HW_PLANE_4    3
#define PMX_HW_PLANE_5    4
#define PMX_HW_PLANE_6    5
#define PMX_HW_PLANE_7    6
#define PMX_HW_PLANE_8    7

#define PMX_HW_VIDEO_MIX  PMX_HW_PLANE_1
#define PMX_HW_OSD1_MIX   PMX_HW_PLANE_3
#define PMX_HW_OSD2_MIX   PMX_HW_PLANE_4
#define PMX_HW_OSD3_MIX   PMX_HW_PLANE_5
#define PMX_HW_OSD4_MIX   PMX_HW_PLANE_7

enum {
	LVDS_SINGLE_LINK=0,
	LVDS_DUAL_LINK=1,
};

typedef  enum
{
    RES_480I=0,
    RES_576I, //1
    RES_480P, //2
    RES_576P, //3
    RES_480P_1440,//4
    RES_576P_1440,//5
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
    RES_2D_640x480HZ,//39
    RES_PANEL_AUO_B089AW01, //40 //Total: 1344x625, Act: 1024x600, Frm: 60Hz, Clk: 50.4MHz
    RES_3D_720P60HZ_TB,  //41
    RES_3D_720P50HZ_TB,  //42
    RES_3D_1080I60HZ_SBS_HALF,//43
    RES_3D_1080I50HZ_SBS_HALF,//44
    RES_3D_1080P23HZ_TB, //45
    RES_3D_1080P24HZ_TB,//46
    
    RES_2160P_23_976HZ, //47
    RES_2160P_24HZ, //48
    RES_2160P_25HZ, //49
    RES_2160P_29_97HZ, //50
    RES_2160P_30HZ,  //51
    RES_2161P_24HZ,  //52

    
    RES_720P30HZ, // 53
    RES_720P25HZ, // 54
    RES_720P24HZ, // 55
    RES_720P23HZ, // 56
//3D frame packet
    RES_3D_1080P60HZ,// 57
    RES_3D_1080P50HZ,//58
    RES_3D_1080P30HZ,//59
    RES_3D_1080P29HZ,//60
    RES_3D_1080P25HZ,//61
    RES_3D_720P24HZ, //62
    RES_3D_720P23HZ, //63
//3D Top and bottom    
    RES_3D_1080P60HZ_TB, //64
    RES_3D_1080P50HZ_TB, //65
    RES_3D_1080P30HZ_TB, //66
    RES_3D_1080P29HZ_TB,//67
    RES_3D_1080P25HZ_TB, //68
    RES_3D_1080I60HZ_TB, //69
    RES_3D_1080I50HZ_TB,//70
    RES_3D_1080I30HZ_TB,//71
    RES_3D_1080I25HZ_TB,//72
    RES_3D_720P30HZ_TB,//73
    RES_3D_720P25HZ_TB,//74
    RES_3D_720P24HZ_TB,//75
    RES_3D_720P23HZ_TB,//76
    RES_3D_576P50HZ_TB,//77
    RES_3D_576I25HZ_TB,//78
    RES_3D_576I50HZ_TB,//79
    RES_3D_480P60HZ_TB,//80
    RES_3D_480I30HZ_TB,//81
    RES_3D_480I60HZ_TB,//82
//3D Side by Side half
    RES_3D_1080P60HZ_SBS_HALF,//83
    RES_3D_1080P50HZ_SBS_HALF,//84
    RES_3D_1080P30HZ_SBS_HALF,//85
    RES_3D_1080P29HZ_SBS_HALF,//86
    RES_3D_1080P25HZ_SBS_HALF,//87
    RES_3D_1080P24HZ_SBS_HALF,//88
    RES_3D_1080P23HZ_SBS_HALF,//89
    RES_3D_1080I30HZ_SBS_HALF,//90
    RES_3D_1080I25HZ_SBS_HALF,//91
    RES_3D_720P60HZ_SBS_HALF,//92
    RES_3D_720P50HZ_SBS_HALF,//93
    RES_3D_720P30HZ_SBS_HALF,//94
    RES_3D_720P25HZ_SBS_HALF,//95
    RES_3D_720P24HZ_SBS_HALF,//96
    RES_3D_720P23HZ_SBS_HALF,//97
    RES_3D_576P50HZ_SBS_HALF,//98
    RES_3D_576I25HZ_SBS_HALF,//99
    RES_3D_576I50HZ_SBS_HALF,//100
    RES_3D_480P60HZ_SBS_HALF,//101
    RES_3D_480I30HZ_SBS_HALF,//102
    RES_3D_480I60HZ_SBS_HALF,//103

	RES_720I,               //104
	
    RES_MODE_NUM,          //105  dummy mode, used to determine the last mode
    RES_AUTO,               //106 

    //add@jgao
    RES_480P_800 =4,//4
    RES_600P_800,
    RES_600P_1024,  
    RES_800P_1280,

    RES_2160P =40,
    RES_480P_800_50HZ,//4
    RES_600P_800_50HZ,      
    RES_600P_1024_50HZ,
}   PMX_RESOLUTION_MODE_T;

// Plane mixer TV type
#define PMX_TV_TYPE_NTSC		         0
#define PMX_TV_TYPE_PAL_M		         1
#define PMX_TV_TYPE_PAL_N		         2
#define PMX_TV_TYPE_PAL			         3
#define PMX_TV_TYPE_PAL_1080P_24		 4
#define PMX_TV_TYPE_PAL_1080P_25		 5
#define PMX_TV_TYPE_PAL_1080P_30		 6
#define PMX_TV_TYPE_NTSC_1080P_23_9		 7 //1080P23.976hz
#define PMX_TV_TYPE_NTSC_1080P_29_9	     8//1080P29.97hz
#define PMX_TV_TYPE_720P3D	             9
#define PMX_TV_TYPE_1080i3D	             10
#define PMX_TV_TYPE_800X480              11
#define PMX_TV_TYPE_800X600              12
#define PMX_TV_TYPE_1024X600             13

/*  pmx_vfy_hal.h */
#define PANEL_SIZE_800_480  0
#define PANEL_SIZE_800_600  1
#define PANEL_SIZE_1024_600 2
#define PANEL_SIZE_1280_720 3
#define PANEL_SIZE_1280_800 4
#define PANEL_SIZE_1920_1080 5 

#define PMX_REALCHIP_EN 1

#define REG_SETTING(setting_data_array) #setting_data_array, setting_data_array, sizeof(setting_data_array)/4

typedef struct
{
    const char *szArrayName;
    unsigned int    *pu4RegSetting;
    unsigned int     size;
} REG_SET_ARM2_T;

extern unsigned int u4SubCvbs480i_arm2[49*2];
extern unsigned int u4SubCvbs480p_arm2[48*2];
extern unsigned int u4SubCvbs576i_arm2[49*2];
extern unsigned int u4SubCvbs576p_arm2[49*2];
extern unsigned int u4SubCav480i_arm2[11*2];
extern unsigned int u4SubCav480p_arm2[11*2];
extern unsigned int u4SubCav576i_arm2[11*2];
extern unsigned int u4SubCav576p_arm2[11*2];
extern unsigned int u4SubCav720p_arm2[20*2];
extern unsigned int u4SubCav1080i_arm2[20*2];
extern unsigned int u4SubCav1080p_arm2[20*2];
extern unsigned int u4SubCav720p50hz_arm2[20*2];
extern unsigned int u4SubCav1080i50hz_arm2[20*2];
extern unsigned int u4SubCav1080p50hz_arm2[20*2];
extern unsigned int ArSclerFrontTiming800x480_arm2[13*2];
extern unsigned int ArSclerFrontTiming720p_arm2[18*2];
extern unsigned int ArSclerFrontTiming1080p_arm2[18*2];
extern unsigned int ArSclerRearTiming480i_arm2[6*2];
extern unsigned int ArSclerRearTiming480p_arm2[9*2];
extern unsigned int ArSclerRearTiming576i_arm2[5*2];
extern unsigned int ArSclerRearTiming576p_arm2[5*2];
extern unsigned int ArSclerRearTiming720i_arm2[5*2];
extern unsigned int ArSclerRearTiming720p_arm2[5*2];
extern unsigned int ArSclerRearTiming1080i_arm2[5*2];
extern unsigned int ArSclerRearTiming1080p_arm2[5*2];
extern unsigned int ArSclerFrontTiming800x600p_arm2[18*2];
extern unsigned int ArSclerFrontTiming1024x600p_arm2[18*2];

extern const char* szResStr[26*4 + 1];

extern REG_SET_ARM2_T _ArCvbsSetting_arm2_T[4];
extern REG_SET_ARM2_T _ArCavSetting_arm2_T[10];
extern REG_SET_ARM2_T _ArSclerTimingSetting_arm2[13];

extern unsigned int u4VdoActMain480_arm2[11*2];
extern unsigned int u4VdoActMain576_arm2[12*2];
extern unsigned int u4VdoActMain720_arm2[14*2];
extern unsigned int u4VdoActMain1080_arm2[15*2];
extern unsigned int u4VdoActSub480_arm2[7*2];
extern unsigned int u4VdoActSub576_arm2[7*2];
extern unsigned int u4VdoActSub720_arm2[8*2];
extern unsigned int u4VdoActSub1080_arm2[7*2];
extern unsigned int u4VdoActMain1080_23_arm2[11*2];
extern unsigned int u4VdoActMain1080_24_arm2[11*2];
extern unsigned int u4VdoActMain1080_25_arm2[11*2];
extern unsigned int u4VdoActMain1080_29_arm2[11*2];
extern unsigned int u4VdoActMain1080_30_arm2[11*2];
extern unsigned int u4VdoActSub1080_23_arm2[11*2];
extern unsigned int u4VdoActSub1080_24_arm2[11*2];
extern unsigned int u4VdoActSub1080_25_arm2[11*2];
extern unsigned int u4VdoActSub1080_29_arm2[11*2];
extern unsigned int u4VdoActSub1080_30_arm2[11*2];
extern unsigned int u4VdoActMain720_50_arm2[12*2];
extern unsigned int u4VdoActMain1080_50_arm2[12*2];
extern unsigned int u4VdoActSub720_50_arm2[7*2];
extern unsigned int u4VdoActSub1080_50_arm2[7*2];
extern unsigned int u4VdoActMainNewSD480_arm2[12*2];
extern unsigned int u4VdoActThird480_arm2[12*2];
extern unsigned int u4VdoActThird720_arm2[12*2];
extern unsigned int u4VdoActThird1080_arm2[11*2];
extern unsigned int u4VdoActThird2160_3840_arm2[11*2];
extern unsigned int u4VdoActThird2161_4096_arm2[11*2];
extern unsigned int u4VdoActThird720_50_arm2[12*2];
extern unsigned int u4VdoActThird1080_23_arm2[11*2];
extern unsigned int u4VdoActThird1080_24_arm2[11*2];
extern unsigned int u4VdoActThird1080_25_arm2[11*2];
extern unsigned int u4VdoActThird1080_29_arm2[11*2];
extern unsigned int u4VdoActThird1080_30_arm2[11*2];
extern unsigned int u4VdoActThird1080_50_arm2[11*2];
extern unsigned int u4VdoActThird576_arm2[12*2];
extern unsigned int u4VdoActSub2160_3840_arm2[11*2];
extern unsigned int u4VdoActSub2161_4096_arm2[11*2];
extern unsigned int u4VdoActMainDSD480_arm2[10*2];
extern unsigned int u4VdoActMain800x480_arm2[15*2];
extern unsigned int u4VdoActMain800x600_arm2[15*2];
extern unsigned int u4VdoActMain1024x600_arm2[15*2];

extern REG_SET_ARM2_T _ArVdoActRegionSetting_arm2[42];

extern unsigned int u4VdoMain480_arm2[26*2];
extern unsigned int u4VdoMain576_arm2[28*2];
extern unsigned int u4VdoMain720_arm2[33*2];
extern unsigned int u4VdoMain1080_arm2[26*2];
extern unsigned int u4VdoSub480_arm2[26*2];
extern unsigned int u4VdoSub576_arm2[27*2];
extern unsigned int u4VdoSub720_arm2[33*2];
extern unsigned int u4VdoSub1080_arm2[26*2];
extern unsigned int u4VdoMainNewSD720_arm2[27*2];
extern unsigned int u4VdoMainNewSD1080_arm2[27*2];
extern unsigned int u4VdoMainNewSD480_arm2[27*2];
extern unsigned int u4VdoThird480_arm2[29*2];
extern unsigned int u4VdoThird720_arm2[29*2];
extern unsigned int u4VdoThird1080_arm2[29*2];
extern unsigned int u4VdoThird2160_arm2[28*2];
extern unsigned int u4VdoThird2161_arm2[29*2];
extern unsigned int u4VdoMainDSD720_arm2[36*2];
extern unsigned int u4VdoMainDSD1080_arm2[35*2];
extern unsigned int u4VdoMainDSD480_arm2[26*2];
extern unsigned int u4VdoMain800x480_arm2[26*2];
extern unsigned int u4VdoMain800x600_arm2[26*2];
extern unsigned int u4VdoMain1024x600_arm2[33*2];

extern REG_SET_ARM2_T _ArVdoSetting_arm2[22];

extern unsigned int u4VdoHScalerH8PhaseCoef0_arm2[17];
extern unsigned int u4VdoHScalerH16PhaseCoef0_arm2[17];
extern unsigned int u4VdoHScalerH8PhaseCoef1_arm2[17];
extern unsigned int u4VdoHScalerH16PhaseCoef1_arm2[17];
extern unsigned int u4VdoHScalerCoef2_arm2[17];
extern unsigned int u4VdoHScalerCoef1_arm2[17];
extern unsigned int u4VdoHScalerCoef4_arm2[17];
extern unsigned int u4VdoHScalerH16PhaseCoef2_arm2[17];
extern unsigned int u4VdoHScalerH16PhaseCoef3_arm2[17];
extern unsigned int u4VdoHScalerH16PhaseCoef4_arm2[17];

extern REG_SET_ARM2_T _ArVdoHCoefSetting_arm2[11];

#define PMX_HAL_MIX_REG_NUM_3365               (0x10/4)
typedef struct _PMX_HAL_MIX_FIELD_T
{
  /* DWORD - 000 */
  unsigned int        fgVIDEO_SRC_SEL                 : 1;
  unsigned int        fgVIDEO_MIX_EN                  : 1;
  unsigned int        fgOSD1_MIX_EN                   : 1;
  unsigned int        fgOSD2_MIX_EN                   : 1;
  unsigned int        fgOSD3_MIX_EN                   : 1;
  unsigned int        fgOSD4_MIX_EN      	      : 1;
  unsigned int        				      : 1;
  unsigned int                                        : 1;
  unsigned int        u4MIX_LAYER1_SEL                : 3;
  unsigned int        fgDST_SEL_1                     : 1;
  unsigned int        u4MIX_LAYER2_SEL                : 3;
  unsigned int        fgDST_SEL_2                     : 1;
  unsigned int        u4MIX_LAYER3_SEL                : 3;
  unsigned int        fgDST_SEL_3                     : 1;
  unsigned int        u4MIX_LAYER4_SEL                : 3;
  unsigned int        fgDST_SEL_4                     : 1;
  unsigned int        u4MIX_LAYER5_SEL                : 3;
  unsigned int        fgDST_SEL_5                     : 1;
  unsigned int                                        : 2;
  unsigned int        fgVIDEO_TIMING_SEL              : 1; 
  unsigned int        fgVIDEO_A_ADJ                   : 1;

  /* DWORD - 004 */
  unsigned int        u4VIDEO_A_IN_RANGE              : 8;
  unsigned int                                        : 8;
  unsigned int        u4VIDEO_A_OUT_RANGE             : 8;
  unsigned int                                        : 8;

  /* DWORD - 008*/
  unsigned int        fgOSD_SRC_SE                    : 1;
  unsigned int        fgOSD_SYNC_FLD_P                : 1;
  unsigned int        fgOSD_SYNC_H_P                  : 1;
  unsigned int        fgOSD_SYNC_V_P                  : 1;
  unsigned int        fgOSD_AUX_SRC_SE                : 1;
  unsigned int        fgOSD_AUX_SYNC_FLD_P            : 1;
  unsigned int        fgOSD_AUX_SYNC_H_P              : 1;
  unsigned int        fgOSD_AUX_SYNC_V_P              : 1;
  unsigned int        fgOSD_R_SRC_SE                  : 1;
  unsigned int        fgOSD_R_SYNC_FLD_P              : 1;
  unsigned int        fgOSD_R_SYNC_H_P                : 1;
  unsigned int        fgOSD_R_SYNC_V_P                : 1;
  unsigned int                                        : 20;

  /* DWORD - 00C*/
  unsigned int        fgINIT_CRC_OSD1                 : 1;
  unsigned int        fgINIT_CRC_OSD2                 : 1;
  unsigned int        fgINIT_CRC_OSD3                 : 1;
  unsigned int        fgINIT_CRC_OSD4                 : 1;
  unsigned int        fgINIT_CRC_OSD5                 : 1;
  unsigned int        fgINIT_CRC_OSD6                 : 1;
  unsigned int        fgINIT_CRC_OSD7                 : 1;
  unsigned int        fgINIT_CRC_FMT_F                : 1;
  unsigned int        fgINIT_CRC_FMT_R                : 1;
  unsigned int        fgINIT_CRC_VDO_F                : 1;
  unsigned int        fgINIT_CRC_VDO_R                : 1;
  unsigned int        fgINIT_CRC_SCL                  : 1;
  unsigned int        fgINIT_CRC_FMT                  : 1;
  unsigned int        fgINIT_CRC_FPD                  : 1;
  unsigned int                                        : 1;
  unsigned int        fgCLR_CRC_OSD1                  : 1;
  unsigned int        fgCLR_CRC_OSD2                  : 1;
  unsigned int        fgCLR_CRC_OSD3                  : 1;
  unsigned int        fgCLR_CRC_OSD4                  : 1;
  unsigned int        fgCLR_CRC_OSD5                  : 1;
  unsigned int        fgCLR_CRC_OSD6                  : 1;
  unsigned int        fgCLR_CRC_OSD7                  : 1;
  unsigned int        fgCLR_CRC_FMT_F                 : 1;
  unsigned int        fgCLR_CRC_FMT_R                 : 1;
  unsigned int        fgCLR_CRC_VDO_F                 : 1;
  unsigned int        fgCLR_CRC_VDO_R                 : 1;
  unsigned int        fgCLR_CRC_SCL                   : 1;
  unsigned int        fgCLR_CRC_FMT                   : 1;
  unsigned int        fgCLR_CRC_FPD                   : 1;
  unsigned int                                        : 1;
}PMX_HAL_MIX_FIELD3365_T;

typedef union _PMX_HAL_MIX_UNION_T
{
  unsigned int              au4Reg[PMX_HAL_MIX_REG_NUM_3365];
  PMX_HAL_MIX_FIELD3365_T    rField;
} PMX_HAL_MIX_UNION_T;

//pmx_hw.h
typedef struct _HAL_PMX_DISP_MAIN_FIELD_T
{
  /* DWORD - 000 */
  unsigned int        u4YHCOEF0               : 32;

  /* DWORD - 004 */
  unsigned int        u4YHCOEF1               : 32;

  /* DWORD - 008 */
  unsigned int        u4YHCOEF2               : 32;

  /* DWORD - 00C */
  unsigned int        u4YHCOEF3               : 32;

  /* DWORD - 010 */
  unsigned int        u4YHCOEF4               : 32;

  /* DWORD - 014 */
  unsigned int        u4YHCOEF5               : 32;

  /* DWORD - 018 */
  unsigned int        u4YHCOEF6               : 32;

  /* DWORD - 01C */
  unsigned int        u4YHCOEF7               : 32;

  /* DWORD - 020 */
  unsigned int        u4YHCOEF8               : 32;

  /* DWORD - 024 */
  unsigned int        u4YHCOEF9               : 32;

  /* DWORD - 028 */
  unsigned int        u4YHCOEF10              : 32;

  /* DWORD - 02C */
  unsigned int        u4YHCOEF11              : 32;

  /* DWORD - 030 */
  unsigned int        u4YHCOEF12              : 32;

  /* DWORD - 034 */
  unsigned int        u4YHCOEF13              : 32;

  /* DWORD - 038 */
  unsigned int        u4YHCOEF14              : 32;

  /* DWORD - 03C */
  unsigned int        u4YHCOEF15              : 32;

  /* DWORD - 040 */
  unsigned int                                : 32;

  /* DWORD - 044 */
  unsigned int                                        : 32;

  /* DWORD - 048 */   // 0x42048
  unsigned int        fgV1_Y                    : 1; // 0
  unsigned int        fgV1_C                    : 1; // 1
  unsigned int        fgV1_NO_SYM           : 1; // 2
  unsigned int        fgV1_SYM_OPT            : 1; // 3
  unsigned int        u1V1_Y_MSB              : 2; // 5:4
  unsigned int        u1V1_C_MSB              : 2; // 7:6
  unsigned int                                : 2;  // 9:8
  unsigned int        fgC_16_PHAEE            : 1;  // 10
  unsigned int        fgC_EVEN_FILTER         : 1;  // 11
  unsigned int                                : 20; // 31:12

  /* DWORD - 04C */
  unsigned int        u4YHCOEF19              : 32;

  /* DWORD - 050 */
  unsigned int        fgCRC_INIT              : 1;
  unsigned int        fgCRC_CLR               : 1;
  unsigned int                                : 6;
  unsigned int        u4CRC_RLT               : 24;

  /* DWORD - 054 */
  unsigned int        u2DERING_THRESHOLD_Y      : 10;
  unsigned int        u2DERING_THRESHOLD_C      : 10;
  unsigned int                                  : 4;
  unsigned int        u1DERING_TRANS        : 4;
  unsigned int        fgDERING_EN_Y             : 1;
  unsigned int        fgDERING_EN_C             : 1;
  unsigned int        fgDERING_EN_EX            : 1;
  unsigned int                                  : 1;

  /* DWORD - 058 */
  unsigned int                                : 16;
  unsigned int        u1BD_X_WDH          : 6;
  unsigned int                                : 2;
  unsigned int        u1BD_Y_WDH          : 6;
  unsigned int                                : 1;
  unsigned int        fgBD_ON             : 1;

  /* DWORD - 05C */
  unsigned int                                        : 2;
  unsigned int        u1BD_Y                  : 6;
  unsigned int                                : 2;
  unsigned int        u1BD_CB             : 6;
  unsigned int                                : 2;
  unsigned int        u1BD_CR             : 6;
  unsigned int                                : 8;

  /* DWORD - 060 */
  unsigned int                                : 32;

  /* DWORD - 064 */
  unsigned int                                : 32;

  /* DWORD - 068 */
  unsigned int                                : 32;

  /* DWORD - 06C */
  unsigned int        fgHDWN_EN               : 1;
  unsigned int                                : 1;
  unsigned int        u1HDWN_TT_MD            : 2;
  unsigned int                                : 1;
  unsigned int        fgFULL_FIFO             : 1;
  unsigned int                                : 2;
  unsigned int        fgHDWN_422          : 1;
  unsigned int        fgSIDE_BLK              : 1;
  unsigned int        fgSIDE_LNR              : 1;
  unsigned int        fgHDWN_444          : 1;
  unsigned int        u1HDWN_OPT          : 4;
  unsigned int        u2HDWN_FAC          : 16;

  /* DWORD - 070 */
  unsigned int        u2HDWN_HEND         : 12;
  unsigned int                                : 4;
  unsigned int        u2HDWN_HBGN         : 12;
  unsigned int                                : 3;
  unsigned int        fgHDWN_ALL              : 1;

  /* DWORD - 074 */
  unsigned int        u2HDWN_VOEND            : 11;
  unsigned int                                : 5;
  unsigned int        u2HDWN_VOBGN            : 11;
  unsigned int                                : 5;

  /* DWORD - 078 */
  unsigned int        u2HDWN_VEEND            : 11;
  unsigned int                                : 5;
  unsigned int        u2HDWN_VEBGN            : 11;
  unsigned int                                : 5;

  /* DWORD - 07C */
  unsigned int        u2HDWN_HO_END       : 13;
  unsigned int        u2SCALE_R_SPEED     : 3;
  unsigned int        u2HDWN_HO_BGN       : 12;
  unsigned int                                : 1;
  unsigned int        u1HDWN_VO_POS       : 3;

  /* DWORD - 080 */
  unsigned int        fgNR_ADJ_SYNC_EN        : 1;
  unsigned int        fgNR_ADJ_FWD            : 1;
  unsigned int        fgNR_TOGGLE_OPT     : 1;
  unsigned int                                : 5;
  unsigned int        u2NR_HSYNC_DELAY        : 12;
  unsigned int        u2NR_VSYNC_DELAY        : 12;

  /* DWORD - 084 */
  unsigned int        u2NR_VSYNC_END      : 11;
  unsigned int                                : 1;
  unsigned int        u2NR_VSYNC_SRT      : 11;
  unsigned int                                : 1;
  unsigned int        fgNR_VSYNC_POLAR        : 1;
  unsigned int        fgNR_HSYNC_POLAR        : 1;
  unsigned int        fgNR_FIELD_POLAR        : 1;
  unsigned int        fgNR_DE_SELF            : 1;
  unsigned int        fgNR_ODD_V_SRT_OPT  : 1;
  unsigned int        fgNR_ODD_V_END_OPT  : 1;
  unsigned int        fgNR_EVEN_V_SRT_OPT : 1;
  unsigned int        fgNR_EVEN_V_END_OPT : 1;

  /* DWORD - 088 */
  unsigned int        u2NR_HOR_END            : 12;
  unsigned int                                : 4;
  unsigned int        u2NR_HOR_SRT            : 12;
  unsigned int                                : 4;


  /* DWORD - 08C */
  unsigned int        u2NR_VO_END         : 11;
  unsigned int                                : 5;
  unsigned int        u2NR_VO_SRT         : 11;
  unsigned int                                : 5;

  /* DWORD - 090 */
  unsigned int        u2NR_VE_END         : 11;
  unsigned int                                : 5;
  unsigned int        u2NR_VE_SRT         : 11;
  unsigned int                                : 5;

  /* DWORD - 094 */ //0x42094
  unsigned int        u1HSYNWIDTH         : 8;
  unsigned int        u1VSYNWIDTH         : 5;
  unsigned int        fgHD_TP                 : 1; //[13] 1:HD 720 format,0:HD 1080 format
  unsigned int        fgHD_ON               : 1; //[14] 1:HD mode(1080i/720p),0:SD mode(480/576)
  unsigned int        fgPRGS                  : 1; //[15] Progressive output enable
  unsigned int                                : 1;
  unsigned int        fgPRGS_AUTOFLD      : 1;
  unsigned int        fgPRGS_INVFLD           : 1;
  unsigned int                                : 1;
  unsigned int        fgYUV_RST_OPT           : 1;
  unsigned int                                : 3;
  unsigned int        fgPRGS_FLD              : 1;
  unsigned int                                :2;
                 //NEW_SD_144M
  unsigned int        fgDSDCLK_EANBLE         :1; 
  unsigned int        fgNEW_SD_MODE       : 1;
  unsigned int        fgNEW_SD_USE_EVEN   : 1;
  unsigned int        u1TVMODE                : 2; //[31:30] TV mode selection 00 NTSC 01 PAL_N 10 PAL_M 11 PAL_BDGHI

  /* DWORD - 098 */
  unsigned int                                : 16;
  unsigned int        u1PF_ADV                : 4;
  unsigned int        u1LUMACONTROL           : 12;

  /* DWORD - 09C */
  unsigned int        u2PXLLEN                : 12;
  unsigned int                                : 20;

  /* DWORD - 0A0 */ // 0x420A0
  unsigned int        u2HACTEND               : 12;
  unsigned int                                : 4;
  unsigned int        u2HACTBGN               : 12;
  unsigned int                                : 1;
  unsigned int        fgLayer3_En             : 1;
  unsigned int        fgPmx_From_NR           : 1;
  unsigned int        fgLayer3_Trigger        : 1;

  /* DWORD - 0A4 */
  unsigned int        u2VOACTEND          : 11;
  unsigned int                                : 5;
  unsigned int        u2VOACTBGN          : 11;
  unsigned int                                : 1;
  unsigned int        u1HIDE_OST              : 4;

  /* DWORD - 0A8 */
  unsigned int        u2VEACTEND              : 11;
  unsigned int                                : 5;
  unsigned int        u2VEACTBGN              : 11;
  unsigned int                                : 1;
  unsigned int        u1HIDE_EST              : 4;

  /* DWORD - 0AC */
  unsigned int        fgVDO_EN                : 1;
  unsigned int        fgFMTM                  : 1;
  unsigned int                                : 1;
  unsigned int        fgHPOR                  : 1;
  unsigned int        fgVPOR                  : 1;
  unsigned int                                : 2;
  unsigned int        fgC_RST_SEL         : 1;
  unsigned int        u4PXLSEL                : 2;
  unsigned int        fgFTRST                 : 1;
  unsigned int        fgSHVSYN                : 1;
  unsigned int                                : 2;
  unsigned int        u1SYN_DEL               : 2;
  unsigned int                                : 3;
  unsigned int        fgUVSW                  : 1;
  unsigned int                                : 5;
  unsigned int        fgBLACK                 : 1;
  unsigned int                                : 1;
  unsigned int        fgPFOFF                 : 1;
  unsigned int        u1HW_OPT                : 4;

  /* DWORD - 0B0 */ // 0x420B0
  unsigned int        fgHSON                  : 1;
  unsigned int        fgHSLR                  : 1;
  unsigned int        fgLPF_ON                : 1;
  unsigned int        fgLPF_SL                : 1;
  unsigned int        u1ED_YUV_ST         : 2;
  unsigned int        fgO_DIS                 : 1;
  unsigned int        fgHDLPF                 : 1;
  unsigned int        u1YACC_ST               : 4;
  unsigned int        u1CACC_ST               : 4;
  unsigned int        u2HSFACTOR              : 14;
  unsigned int        fgEVEN_FIR              : 1;
  unsigned int        fgPHASE_16              : 1;

  /* DWORD - 0B4 */
  unsigned int                                : 4;
  unsigned int        u1BIY                   : 4;
  unsigned int                                : 4;
  unsigned int        u1BICB                  : 4;
  unsigned int                                : 4;
  unsigned int        u1BICR                  : 4;
  unsigned int        fgPF2OFF                : 1;
  unsigned int        fgHIDE_L                : 1;
  unsigned int                                : 6;

  /* DWORD - 0B8 */
  unsigned int                                : 4;
  unsigned int        u1BGY                   : 4;
  unsigned int                                : 4;
  unsigned int        u1BGCB                  : 4;
  unsigned int                                : 4;
  unsigned int        u1BGCR                  : 4;
  unsigned int                                : 8;

  /* DWORD - 0BC */
  unsigned int        u1EDGE_RATIO            : 5;
  unsigned int        fgKNEE                  : 1;
  unsigned int        fgZCORE                 : 1;
  unsigned int        fgDNR                   : 1;
  unsigned int        u1CORE                  : 4;
  unsigned int        u1DOUT_CTL          : 3;
  unsigned int                                : 17;

  /* DWORD - 0C0 */
  unsigned int        u2DowndScaleEnd       : 16;
  unsigned int        u2DowndScaleStart     : 16;
  /* DWORD - 0C4 */
  unsigned int        u2C4Default             : 16;
  unsigned int        fgOLD_C_ACC         : 1;
  unsigned int                                : 3;
  unsigned int        fgTVE_ND                : 1;
  unsigned int                                : 3;
  unsigned int        u1FIRST_PXL_LEAD        : 8;

  /* DWORD - 0C8 */
  unsigned int        NEW_SCL_MODE_EN         : 1;
  unsigned int        POST_SCL_USE_AC         : 1;
  unsigned int                                : 14;
  unsigned int        POST_SCL_WINDOW_LINEAR_SIZE_SEL : 2;
  unsigned int        POST_SCL_WINDOW_ACC_SIZE_SEL : 2;
  unsigned int        POST_SCL_PRE_DATA_NEXT_LUMA_Y_OPTION : 1; // 2'b00:2, 2'b01:4, 2'b10:6, 2'b11:8
  unsigned int        POST_SCL_NEXT_DATA_PRE_LUMA_Y_OPTION : 1;
  unsigned int        POST_SCL_PRE_DATA_NEXT_LUMA_C_OPTION : 1;
  unsigned int        POST_SCL_NEXT_DATA_PRE_LUMA_C_OPTION : 1;
  unsigned int                                : 7;
  unsigned int        POST_DIV2_SEL           : 1;

  // 0x420CC
  unsigned int        u4ScalerFactor          : 16;
  unsigned int        fgDemoModeEnable    : 1; //[16]
  unsigned int        fgDemoModeLeft      : 1; //[17]
  unsigned int                                        : 2;
  unsigned int        u2DemoModeWidth     : 12;

 // 0x420D0
  /* DWORD - 0D0 */
  unsigned int        u2V_TOTAL_MIX           : 12;
  unsigned int                                : 4;
  unsigned int        u2H_TOTAL_MIX           : 13;
  unsigned int                                : 2;
  unsigned int        fgADJ_T_MIX             : 1;

  /* DWORD - 0D4 */ // 0x420D4
  unsigned int        u2V_TOTAL               : 12;
  unsigned int        horizontal_use_4fs      : 1;
  unsigned int        horizontal_use_3fs      : 1;
  unsigned int        horizontal_use_2fs      : 1;
  unsigned int        new_hd                  : 1;  
  unsigned int        u2H_TOTAL               : 13;
  unsigned int                                : 1;
  unsigned int        horizontal_use_fs       : 1;  
  unsigned int        fgADJ_T                 : 1;

  /* DWORD - 0D8 */
  unsigned int        INC_END                 : 12;
  unsigned int        INC_START               : 12;
  unsigned int        INC_STEP                : 8;

  /* DWORD - 0DC */
  unsigned int      DEC_END                   : 12;
  unsigned int        DEC_START               : 12;
  unsigned int        INIT_FACTOR             : 8;

  /* DWORD - 0E0 */
  unsigned int        FORCE_NEW_SCL_MODE_PATH_CLOCK_EN : 1;
  unsigned int        FORCE_OLD_SCL_MODE_PATH_CLOCK_EN : 1;
  unsigned int                                : 30;

  /* DWORD - 0E4 */
  unsigned int        fgMULTI_RATIO           : 1;
  unsigned int        fgDIRECT                : 1;
  unsigned int        fgC_FMTRST_M2           : 1;
  unsigned int        fgC_CHK_A_SCA           : 1;//0x423E4[3] luma key option
  unsigned int        fgY_ALL_8TAP_OUT        : 1;
  unsigned int        fgFACT_PREC             : 1;
  unsigned int        fgNOT_PST_D2            : 1;
  unsigned int        fgC_ALL_8TAP_OUT        : 1;
  unsigned int        fgHD_C_FIR_EN           : 1;
  unsigned int        u1LPF_SEL               : 3;
  unsigned int        fgUsePhase32            : 1;
  unsigned int        fgHD_C_POS              : 1;
  unsigned int        fgRST_PHASE         : 1;
  unsigned int        fgDemoModeHoriEnable   : 1;
  // MULTI_EXTEND[7:0]
  unsigned int        INIT_FACTOR_0           : 1; // bit 16
  unsigned int        INIT_FACTOR_9           : 1; // bit 17
  unsigned int        INC_STEP_0              : 1; // bit 18
  unsigned int                                : 1; // bit 19
  unsigned int        DEC_END_12              : 1; // bit 20
  unsigned int        DEC_START_12            : 1; // bit 21
  unsigned int        INC_END_12              : 1; // bit 22
  unsigned int        INC_START_12            : 1; // bit 23
  unsigned int        fgMVDO_4TAP_ONLY        : 1;
  unsigned int        fgMATCH_2T              : 1;
  unsigned int        fgOLD_NEXT_YC           : 1;
  unsigned int        fgNO_SUB_1              : 1;
  unsigned int        fgEN_FIRST_PXL_LEAD : 1;
  unsigned int        fgDownScaleMode         : 1;
  unsigned int        fg8TapCSControl         : 1;
  unsigned int                                : 1;

//0x420E8
  unsigned int        u2HSYN_DELAY            : 12;
  unsigned int                                : 4;
  unsigned int        u2VSYN_DELAY            : 12;
  unsigned int                                : 3;
  unsigned int        fgADJ_FWD               : 1;
  unsigned int        u2DSDHSYN_DELAY         : 13;
  unsigned int                                : 3;
  unsigned int        u2DSDVSYN_DELAY         : 13;
  unsigned int                                : 2;
  unsigned int        fgDSDADJ_FWD                : 1;
  /* DWORD - 0F0 */
  unsigned int                                : 32;

  /* DWORD - 0F4 */
  unsigned int                                : 32;

  /* DWORD - 0F8 */
  unsigned int        fgCCONV_VP1         : 1;
  unsigned int        fgCC7TO6_VP1            : 1;
  unsigned int                                : 2;
  unsigned int        fgAverage422to444       : 1;
  unsigned int        fgOLD_CHROMA            : 1;
  unsigned int        fg235_TO_255_EN         : 1;
  unsigned int        fgDATA_235_255          : 1;
  unsigned int                                : 20;
  unsigned int     fgAdap_Chroma              : 1;//[28]
  unsigned int                                : 3;

  /* DWORD - 0FC */
  unsigned int        u1LUMA_KEY              : 12;
  unsigned int                                : 4;
  unsigned int        WINDOW_LINEAR_SIZE_SEL  : 2;
  unsigned int        WINDOW_ACC_SIZE_SEL     : 2;
  unsigned int        PRE_DATA_NEXT_LUMA_Y_OPTION : 1;
  unsigned int        NEXT_DATA_PRE_LUMA_Y_OPTION : 1;
  unsigned int        PRE_DATA_NEXT_LUMA_C_OPTION : 1;
  unsigned int        NEXT_DATA_PRE_LUMA_C_OPTION : 1;
  unsigned int                                : 7;
  unsigned int        POST_DIV2_SEL_2         : 1;
} HAL_PMX_DISP_MAIN_FIELD_T;

#define HAL_PMX_DISP_MAIN_REG_NUM     (0x100/4) //modify for MT8520 12
typedef union _HAL_PMX_DISP_MAIN_UNION_T
{
  unsigned int              au4Reg[HAL_PMX_DISP_MAIN_REG_NUM];
  HAL_PMX_DISP_MAIN_FIELD_T   rField;
} HAL_PMX_DISP_MAIN_UNION_T;

typedef struct _HAL_PMX_DISP_MAIN_C_FIELD_T
{
  /* DWORD - 000 */
  unsigned int        u4CHCOEF0               : 32;

  /* DWORD - 004 */
  unsigned int        u4CHCOEF1               : 32;

  /* DWORD - 008 */
  unsigned int        u4CHCOEF2               : 32;

  /* DWORD - 00C */
  unsigned int        u4CHCOEF3               : 32;

  /* DWORD - 010 */
  unsigned int        u4CHCOEF4               : 32;

  /* DWORD - 014 */
  unsigned int        u4CHCOEF5               : 32;

  /* DWORD - 018 */
  unsigned int        u4CHCOEF6               : 32;

  /* DWORD - 01C */
  unsigned int        u4CHCOEF7               : 32;

  /* DWORD - 020 */
  unsigned int        u4CHCOEF8               : 32;

  /* DWORD - 024 */
  unsigned int        u4CHCOEF9               : 32;

  /* DWORD - 028 */
  unsigned int        u4CHCOEF10              : 32;

  /* DWORD - 02C */
  unsigned int        u4CHCOEF11              : 32;

  /* DWORD - 030 */
  unsigned int        u4CHCOEF12              : 32;

  /* DWORD - 034 */
  unsigned int        u4CHCOEF13              : 32;

  /* DWORD - 038 */
  unsigned int        u4CHCOEF14              : 32;

  /* DWORD - 03C */
  unsigned int        u4CHCOEF15              : 32;

  /* DWORD - 040 */
  unsigned int                                : 32;

  /* DWORD - 044 */
  unsigned int                                        : 32;

  /* DWORD - 048 */
  unsigned int                                : 32;

  /* DWORD - 04C */
  unsigned int        u4CHCOEF19              : 32;
}HAL_PMX_DISP_MAIN_C_FIELD_T;

#define HAL_PMX_DISP_MAIN_C_REG_NUM     (0x50/4)
typedef union _HAL_PMX_DISP_MAIN_C_UNION_T
{
  unsigned int              au4Reg[HAL_PMX_DISP_MAIN_C_REG_NUM];
  HAL_PMX_DISP_MAIN_C_FIELD_T   rField;
} HAL_PMX_DISP_MAIN_C_UNION_T;

#define HAL_PMX_SCLER_MAIN_REG_NUM     (0x100/4) //jg:fixme
#define HAL_PMX_SCLER_AUX_REG_NUM     (0x100/4) //jg:fixme
#define HAL_PMX_DISP_AUX_REG_NUM      (0x100/4) //modify for MT8520 8
#define HAL_PMX_VDOUT_MAIN_REG_NUM     (0x100/4) //modify for MT8520 12
#define HAL_PMX_VDOUT_AUX_REG_NUM      (0x100/4) //modify for MT8520 8
#define HAL_PMX_DISP_AUX_C_REG_NUM     (0x50/4)

#define HAL_WRITE32(_reg_, _val_)  (*((volatile unsigned int*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)          (*((volatile unsigned int*)(_reg_)))

#define IO_READ32(base, offset)    HAL_READ32((base) + (offset))
#define IO_WRITE32(base, offset, value)  HAL_WRITE32((base) + (offset), (value))

#endif
