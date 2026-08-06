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

/*****************************************************************************
*  Audio Driver: Exported interfaces for dspctrl and other modules
*****************************************************************************/

#ifndef _AUD_DRV_H_
#define _AUD_DRV_H_

#include <linux/miscdevice.h>
#include <linux/types.h>

#ifndef EMULATION_FPGA
//#include "x_typedef.h"
#include <linux/types.h>
#else
#include "basetsd.h"
#include "general.h"
#endif
#include <media/atc/drv_aud.h>
#include "chip_ver.h"
#include <media/atc/ose_mem.h>
#include "aud_comm_os.h"
#include "aud_ioctrl.h"
#include <media/atc/aud_output.h>

#include <asm/io.h>
#include "aud_drv_config.h"


// *********************************************************************
// Type definitions
// *********************************************************************
struct adec_dev_info {
	struct miscdevice cdev;
	struct device *dev;
	phys_addr_t adspm_base;
	phys_addr_t adspm_size;
	phys_addr_t afifom_base;
	phys_addr_t afifom_size;
	phys_addr_t adspm_base_va;
	phys_addr_t afifom_base_va;
};

extern struct adec_dev_info *adec_dev;

typedef enum
{
    AUD_DRV_UNINITIALIZED = 0,
    AUD_DRV_TRIGGER_ADSP,
    AUD_DRV_OPENING,
    AUD_DRV_STOPPING,
    AUD_DRV_STOPPED,
    AUD_DRV_PLAYING,
    AUD_DRV_PLAYED,
    AUD_DRV_PAUSING,
    AUD_DRV_PAUSED,
    AUD_DRV_RESUMING,
    AUD_DRV_PRESTARTING,
    RISC_INIT,    //used only on EMULATION_FPGA
    RISC_POWERDOWN,
    AUD_DRV_STATE_CNT
}   AUD_DRV_STATE_T;

typedef enum _DECODER_STATE_T
{
    AUD_DEC_STOP = 0,
    AUD_DEC_PLAYING,
    AUD_DEC_INIT,
    AUD_DEC_PAUSING,
    AUD_DEC_PAUSED,
    AUD_DEC_RESUMING
}   DECODER_STATE_T;

typedef enum
{
    AUD_ESM_EVENT_NONE       = 0,
    AUD_ESM_EVENT_AU_PRI     = 1 << 0,                    // 1
    AUD_ESM_EVENT_EOS        = 1 << 3,
    AUD_ESM_EVENT_AU_SEC     = 1 << 4,
    AUD_ESM_EVENT_AU_TER     = 1 << 5,
    AUD_ESM_EVENT_AU_LINEIN1 = 1 << 6,
    AUD_ESM_EVENT_AU_LINEIN2 = 1 << 7,
}AUD_ESM_EVENT_T;

typedef struct _MEM_BUFFER_INFO_T
{
	u8* pData;
	u32 u4Length;
} MEM_BUFFER_INFO_T;

typedef enum
{
    AV_SYNC_FREE_RUN = 0, // no need to syn
    AV_SYNC_SLAVE,        // syn to STC
    AV_SYNC_AUDIO_MASTER, // update A-PTS to A-STC, may use in PCR or HardDisk playback
}   AV_SYNC_MODE_T;

typedef enum
{
    AV_SYNC_STC_A1 = 0,
    AV_SYNC_STC_A2,
    AV_SYNC_STC_V1,
    AV_SYNC_STC_V2,
}   AV_SYNC_STC_SELECT_T;

typedef struct _AUD_DECODER_T
{
    AUD_DRV_FMT_T            eDecFormat;
    AUD_DRV_STREAM_FROM_T    eStreamFrom;
    MEM_BUFFER_INFO_T        rMemBuffer;
    AV_SYNC_MODE_T           eSynMode;
    AV_SYNC_STC_SELECT_T     eStcId;
    DECODER_STATE_T          eDecState;
    bool                     fgOperationMode; // FALSE: push mode TRUE:pull mode
    u32                      u4EventFlag;
    AUD_DRV_NFY_INFO_T       rNfyInfo;
} AUD_DECODER_T;

typedef enum _ENCODER_STATE_T
{
    AUD_ENC_INIT = 0,
    AUD_ENC_START,
    AUD_ENC_STARTING,
    AUD_ENC_STOP,
    AUD_ENC_STOPPING,
}  ENCODER_STATE_T;

typedef struct _AUD_ENCODER_T
{
    AUD_DRV_FMT_T                   eEncFormat;
    ENCODER_STATE_T                 eEncState;
    AUD_DRV_NFY_INFO_T              rNfyInfo;
    u32                          u4EventFlag;
}AUD_ENCODER_T;

//system level afifo arrange
#define SYS_AFIFO_PART_FST_ADSP_BASE        0
#define SYS_AFIFO_PART_FST_ADSP_SIZE        0x180000
#define SYS_AFIFO_PART_SND_HDMI_BASE        (SYS_AFIFO_PART_FST_ADSP_BASE + SYS_AFIFO_PART_FST_ADSP_SIZE)
#define SYS_AFIFO_PART_SND_HDMI_SIZE        0x200000
#define SYS_AFOFO_PART_THD_PCM_BASE         (SYS_AFIFO_PART_SND_HDMI_BASE + SYS_AFIFO_PART_SND_HDMI_SIZE)
#define SYS_AFOFO_PART_THD_PCM_SIZE         0x80000
#define SYS_AFIFO_MAX_SIZE                  (SYS_AFIFO_PART_FST_ADSP_SIZE + SYS_AFIFO_PART_SND_HDMI_SIZE + SYS_AFOFO_PART_THD_PCM_SIZE)

//audio driver afifo arrange
#define AUD_DRV_AFIFO_MM_SIZE               (0x104000)
#define AUD_DRV_AVIN_A2DP_SZIE              (0x6000 * 3)
#define AUD_DRV_AVIN2_SIZE                  (0x6000)

#define MT3360_ADSP_BUF_SZ                  (0x600000)
#define MT3360_ADSP_BUF_PHY                 (adec_dev->adspm_base)
#define MT3360_DSP_WORKING_BUF              (adec_dev->adspm_base_va)
#define MT3360_AFIFO_PA                     (adec_dev->afifom_base)
#define MT3360_AFIFO_VA                     (adec_dev->afifom_base_va)
#define AUD_AFIFO_PRIMARY_SIZE              AUD_DRV_AFIFO_MM_SIZE

#define AUD_AFIFO_AVIN_A2DP_SIZE            AUD_DRV_AVIN_A2DP_SZIE
#define AUD_AFIFO_AVIN2_SIZE                AUD_DRV_AVIN2_SIZE

#define AUD_HDMI_RX_BUF_PA                  (adec_dev->afifom_base + SYS_AFIFO_PART_SND_HDMI_BASE)
#define AUD_HDMI_RX_BUF_VA                  (MT3360_AFIFO_VA + SYS_AFIFO_PART_SND_HDMI_BASE)
#define AUD_HDMI_RX_BUF_SIZE                (0x18c000)

#define AC83XX_PCM_VA                       (MT3360_AFIFO_VA + SYS_AFOFO_PART_THD_PCM_BASE)
#define AC83XX_PCM_PA                       (adec_dev->afifom_base + SYS_AFOFO_PART_THD_PCM_BASE)

#define AUD_AFIFO_TOTAL_SIZE                (AUD_AFIFO_PRIMARY_SIZE+ AUD_AFIFO_AVIN_A2DP_SIZE+ AUD_AFIFO_AVIN2_SIZE)

#define AFIFO_PHYSICAL(VirAdr)	            (VirAdr - MT3360_AFIFO_VA + MT3360_AFIFO_PA)
#define AFIFO_VIRTUAL(PhyAdr)               (PhyAdr - MT3360_AFIFO_PA + MT3360_AFIFO_VA)

#define ADSP_PHYSICAL(VirAdr)               (VirAdr - MT3360_DSP_WORKING_BUF + MT3360_ADSP_BUF_PHY)
#define ADSP_VIRTUAL(PhyAdr)                (PhyAdr - MT3360_ADSP_BUF_PHY + MT3360_DSP_WORKING_BUF)

#define HDMI_RX_PHYSICAL(VirAdr)            (VirAdr - AUD_HDMI_RX_BUF_VA + AUD_HDMI_RX_BUF_PA)
#define HDMI_RX_VIRTUAL(PhyAdr)             (PhyAdr - AUD_HDMI_RX_BUF_PA + AUD_HDMI_RX_BUF_VA)


typedef struct _AUD_FUNC_OPTION
{
	u32 u4FuncOption0;		//default: 0
	u32 u4FuncOption1;		//default: 0
	u32 u4FuncOption2;		//default: 0

	u32 u4BassCutOffFreq;	//default: 100 (Hz)

	u32 u4GainAvIn;			//default: 0x20000
	u32 u4GainUSB;			//default: 0x20000
	u32 u4GainDVD;			//default: 0x20000

	u32 u4Reserve1;			//default: 0
	u32 u4Reserve2;			//default: 0
	u32 u4Reserve3;			//default: 0
	u32 u4Reserve4;			//default: 0
	u32 u4Reserve5;			//default: 0
	u32 u4Reserve6;			//default: 0

}AUD_FUNC_OPTION_T;



typedef struct _AUD_THRESHOLD
{
	u32 u4FrontThrshld;
	u32 u4RearThrshld;
	u32 u4WaveFormThrshld;
}AUD_THRESHOLD_T;

typedef enum
{
    AUD_SPDIF_LPCM_48K = 0,
    AUD_SPDIF_LPCM_96K,
    AUD_SPDIF_LPCM_192K
}   AUD_PCM_MAX_SF_T;

typedef enum
{
   LPCM_DVD_CH_MONO = 0,
   LPCM_DVD_CH_STEREO = 1,
   LPCM_DVD_CH_LF_RF_S = 2,
   LPCM_DVD_CH_LF_RF_LS_RS = 3,
   LPCM_DVD_CH_LF_RF_C = 7,
   LPCM_DVD_CH_LF_RF_C_S = 8,
   LPCM_DVD_CH_LF_RF_LS_RS_C = 9,
   LPCM_DVD_CH_LF_RF_C_LFE = 0xF,
   LPCM_DVD_CH_LF_RF_LS_RS_C_LFE = 0xC,
   LPCM_DVD_CH_LF_RF_LS_RS_C_LFE_Ch7_Ch8 = 0x17,
   LPCM_DVD_CH_INVALID
}AUD_LPCM_DVD_CHANNEL_ASSIGN_T;

typedef enum
{
   LPCM_DVD_SAMPLING_RATE_48KHZ = 0,
   LPCM_DVD_SAMPLING_RATE_96KHZ = 1,
   LPCM_DVD_SAMPLING_RATE_192KHZ = 2,
   LPCM_DVD_SAMPLING_RATE_24KHZ = 3,
   LPCM_DVD_SAMPLING_RATE_12KHZ = 4,
   LPCM_DVD_SAMPLING_RATE_44KHZ = 8,
   LPCM_DVD_SAMPLING_RATE_88KHZ = 9,
   LPCM_DVD_SAMPLING_RATE_176KHZ = 0xA,
   LPCM_DVD_SAMPLING_RATE_22KHZ = 0xB,
   LPCM_DVD_SAMPLING_RATE_11KHZ = 0xC,
   LPCM_DVD_SAMPLING_RATE_32KHZ = 0x10,
   LPCM_DVD_SAMPLING_RATE_16KHZ = 0x13,
   LPCM_DVD_SAMPLING_RATE_8KHZ = 0x14,
   LPCM_DVD_SAMPLING_RATE_RESERVED
}AUD_LPCM_DVD_SAMPLING_RATE_T;

typedef enum
{
   SPK_FL_FR =   (1<< 0),
   SPK_LFE =     (1<< 1),
   SPK_FC =      (1<< 2),
   SPK_LS_RS =   (1<< 3),
   SPK_RC =      (1<< 4),
   SPK_FLC_FRC = (1<< 5),
   SPK_RLC_RRC = (1<< 6)
} AUD_HDMI_SINK_SPEAK_ALLOCAT;

typedef struct
{
    AUD_DRV_PCM_INFO_T tPcmInfo;
    u8 u1BitResolution;
    u32 u4SamplingFreq;
    u8 u1ChannelAssign;
    u8 u1DecType;
    u8 u1IsBD;
} AUD_DRV_PCM_SETTING_T;



/* Dolby Digital compression mode. */
typedef enum
{
    AUD_DRV_CMPSS_MODE_RF_OFF = 0,
    AUD_DRV_CMPSS_MODE_RF_FULL,
    AUD_DRV_CMPSS_MODE_LINE_0,  // Off
    AUD_DRV_CMPSS_MODE_LINE_1,  // 1/8
    AUD_DRV_CMPSS_MODE_LINE_2,
    AUD_DRV_CMPSS_MODE_LINE_3,
    AUD_DRV_CMPSS_MODE_LINE_4,
    AUD_DRV_CMPSS_MODE_LINE_5,
    AUD_DRV_CMPSS_MODE_LINE_6,
    AUD_DRV_CMPSS_MODE_LINE_7,
    AUD_DRV_CMPSS_MODE_LINE_8,   // Full
    AUD_DRV_CMPSS_MODE_CUSTOM0_0,  // Off
    AUD_DRV_CMPSS_MODE_CUSTOM0_1,  // 1/8
    AUD_DRV_CMPSS_MODE_CUSTOM0_2,
    AUD_DRV_CMPSS_MODE_CUSTOM0_3,
    AUD_DRV_CMPSS_MODE_CUSTOM0_4,
    AUD_DRV_CMPSS_MODE_CUSTOM0_5,
    AUD_DRV_CMPSS_MODE_CUSTOM0_6,
    AUD_DRV_CMPSS_MODE_CUSTOM0_7,
    AUD_DRV_CMPSS_MODE_CUSTOM0_8,   // Full
    AUD_DRV_CMPSS_MODE_CUSTOM1_0,  // Off
    AUD_DRV_CMPSS_MODE_CUSTOM1_1,  // 1/8
    AUD_DRV_CMPSS_MODE_CUSTOM1_2,
    AUD_DRV_CMPSS_MODE_CUSTOM1_3,
    AUD_DRV_CMPSS_MODE_CUSTOM1_4,
    AUD_DRV_CMPSS_MODE_CUSTOM1_5,
    AUD_DRV_CMPSS_MODE_CUSTOM1_6,
    AUD_DRV_CMPSS_MODE_CUSTOM1_7,
    AUD_DRV_CMPSS_MODE_CUSTOM1_8   // Full
}   AUD_DRV_CMPSS_MODE_T;

/* Dolby Digital karaoke mode. */
typedef enum
{
    DSP_AC3_KARA_DISABLE = 0x0001,
    DSP_AC3_KARA_AWARE   = 0x0002,
    DSP_AC3_KARA_NONE    = 0x0004,
    DSP_AC3_KARA_V1      = 0x0008,
    DSP_AC3_KARA_V2      = 0x0010,
    DSP_AC3_KARA_BOTH    = 0x0020,
    DSP_AC3_NO_MELODY    = 0x0080
}   AUD_DRV_DOLBY_KARA_MODE_T;

/* DTS DRC Setting */
typedef enum
{
    AUD_DRV_DTS_DRC_OFF = 0,
    AUD_DRV_DTS_DRC_MODE_0,
    AUD_DRV_DTS_DRC_MODE_1,
    AUD_DRV_DTS_DRC_MODE_2,
    AUD_DRV_DTS_DRC_MODE_3,
    AUD_DRV_DTS_DRC_MODE_4,
    AUD_DRV_DTS_DRC_MODE_5,
    AUD_DRV_DTS_DRC_MODE_6,
    AUD_DRV_DTS_DRC_MODE_7,
    AUD_DRV_DTS_DRC_MODE_8,
    AUD_DRV_DTS_DRC_MODE_9
#if 0
    AUD_DRV_DTS_DRC_BIT = 0,        // Bit 0 : DRC On/Off
    AUD_DRV_DTS_DTSCD_BIT,          // Bit 1 : DTSCD format (14 bit mode)
    AUD_DRV_DTS_DTSES_BIT,          // Bit 2 : DTSES ON/ OFF
    AUD_DRV_DTS_DISCRETE_BIT,       // Bit 3 : DISCRETE/ MATRIX
    AUD_DRV_DTS_96K24B_BIT,         // Bit 4 : 96KHz ON/ OFF
    AUD_DRV_DTS_HIGHRES_BIT,        // Bit 5 : DTS-HD High Resolution support
    AUD_DRV_DTS_MASTERAUD_BIT,      // Bit 6 : DTS-HD Master Audio support
    AUD_DRV_DTS_AUD_PRESENTATION    // Bit 7 : DTS Audio Presentation Select
#endif
}   AUD_DRV_DTS_DRC_MODE_T;



/* Sample frequency. */
typedef enum
{
    AUD_DRV_SAMPLE_FREQ_8 = 0,
    AUD_DRV_SAMPLE_FREQ_11_025,
    AUD_DRV_SAMPLE_FREQ_12,
    AUD_DRV_SAMPLE_FREQ_16,
    AUD_DRV_SAMPLE_FREQ_22_050,
    AUD_DRV_SAMPLE_FREQ_24,
    AUD_DRV_SAMPLE_FREQ_32,
    AUD_DRV_SAMPLE_FREQ_44_100,
    AUD_DRV_SAMPLE_FREQ_48,
    AUD_DRV_SAMPLE_FREQ_96
}   AUD_DRV_SAMPLE_FREQ_T;


/*
0: IEC select L/R as output
1: IEC select Ls/Rs as output
2: IEC select C/LFE as output
3 :IEC select Ch7/Ch8 as output
4: IEC select SPDIF /LINE in as output
5: IEC select Ch9/Ch10 as output
6: IEC select Ch11/Ch12 as output
7: IEC select microphone as output
*/
typedef enum
{
    AUD_DRV_SPDIF_OUTPUT_L_R = 0,
    AUD_DRV_SPDIF_OUTPUT_LS_RS,
    AUD_DRV_SPDIF_OUTPUT_C_LFE,
    AUD_DRV_SPDIF_OUTPUT_CH7_CH8,
    AUD_DRV_SPDIF_OUTPUT_LINE_IN,
    AUD_DRV_SPDIF_OUTPUT_CH9_CH10,
    AUD_DRV_SPDIF_OUTPUT_CH11_CH12,
    AUD_DRV_SPDIF_OUTPUT_MIC
} AUD_DRV_SPDIF_OUTPUT_T;

typedef enum
{
    AUD_MIRA_LOWTH = 0,
    AUD_MIRA_LIGHTH,
    AUD_MIRA_ADJSIZE,
    AUD_MIRA_SLEEPTIME
}AUD_MIRACAST_PARAM_E;

typedef struct _AUD_MIRACAST_PARAM
{
    AUD_MIRACAST_PARAM_E e_ParamID;

    union{
        s64 i8LowThVal;
        s64 i8HighThVal;
        u32 u4AdjustSize;
        u16 u2SleepTime;
        }uVal;
}AUD_MIRACAST_PARAM_T;


/************test tone variable********/
typedef struct _AUD_DEC_TT_INFO_T
{
    bool                       b_testtone_enable;
    AUD_DEC_TEST_TONE_TYPE_T   e_testtone_type;
    u32                     ui4_testtone_freq;
    AUD_DEC_TESTTONE_OUTPUT_T  t_testtone_output;
}   AUD_DEC_TT_INFO_T;

typedef struct
{
  u32 AE_TT_ANALOG_FRONT_RESERVE_0:           1;      //bit0
  u32 AE_TT_ANALOG_FRONT_RESERVE_1:           1;      //bit1
  u32 AE_TT_ANALOG_FRONT_L_SEL_SIG2:          1;      //bit2
  u32 AE_TT_ANALOG_FRONT_R_SEL_SIG2:          1;        //bit3
  u32 AE_TT_ANALOG_FRONT_C_SEL_SIG2:          1;      //bit4
  u32 AE_TT_ANALOG_FRONT_CH7_SEL_SIG2:        1;  //bit5
  u32 AE_TT_ANALOG_FRONT_LS_SEL_SIG2:         1;  //bit6
  u32 AE_TT_ANALOG_FRONT_RS_SEL_SIG2:         1;  //bit7
  u32 AE_TT_ANALOG_FRONT_SW_SEL_SIG2:         1;  //bit8
  u32 AE_TT_ANALOG_FRONT_CH8_SEL_SIG2:        1;  //bit9
  u32 AE_TT_ANALOG_FRONT_CH9_SEL_SIG2:        1;  //bit10
  u32 AE_TT_ANALOG_FRONT_CH10_SEL_SIG2:       1;  //bit11
  u32 AE_TT_ANALOG_FRONT_SW_LPF_EN:           1;  //bit12
  u32 AE_TT_ANALOG_FRONT_SW_RESET:            1;  //bit13
  u32 AE_TT_ANALOG_FRONT_L_EN:                1;   //bit14
  u32 AE_TT_ANALOG_FRONT_R_EN:                1;  //bit15
  u32 AE_TT_ANALOG_FRONT_C_EN:                1;  //bit16
  u32 AE_TT_ANALOG_FRONT_CH7_EN:              1;  //bit17
  u32 AE_TT_ANALOG_FRONT_LS_EN:               1;  //bit18
  u32 AE_TT_ANALOG_FRONT_RS_EN:               1;  //bit19
  u32 AE_TT_ANALOG_FRONT_SW_EN:               1;   //bit20
  u32 AE_TT_ANALOG_FRONT_CH8_EN:              1;  //bit21
  u32 AE_TT_ANALOG_FRONT_CH9_EN:              1;  //bit22
  u32 AE_TT_ANALOG_FRONT_CH10_EN:             1;  //bit23
} AUDIO_TT_FRONT_SWITCH_T;

typedef struct
{
  u32 AE_TT_ANALOG_REAR_RESERVE_0:        1;      //bit0
  u32 AE_TT_ANALOG_REAR_RESERVE_1:        1;      //bit1
  u32 AE_TT_ANALOG_REAR_L_SEL_SIG2:       1;      //bit2
  u32 AE_TT_ANALOG_REAR_R_SEL_SIG2:       1;      //bit3
  u32 AE_TT_ANALOG_REAR_RESERVE_2:        10;     //bit4~bit13
  u32 AE_TT_ANALOG_REAR_L_EN:             1;      //bit14
  u32 AE_TT_ANALOG_REAR_R_EN:             1;  //bit15
} AUDIO_TT_REAR_SWITCH_T;

typedef struct
{
  u32 AE_TT_EN:1;      //bit0
  u32 AE_TT_RESET:    1;      //bit1
  u32 AE_TT_RESERVED1:   1;      //bit2
  u32 AE_TT_RESERVED2: 1;      //bit3
  u32 AE_TT_1st_PINKNOISE:  1;      //bit4
  u32 AE_TT_1st_TRIANGLE:   1;  //bit5
  u32 AE_TT_1st_SINEWAVE:     1;  //bit6
  u32 AE_TT_1st_WHITENOISE_INS:       1;  //bit7
  u32 AE_TT_1st_WHITENOISE_SEED:      1;  //bit8
  u32 AE_TT_1st_PINKNOISE_BOLBY:       1;  //bit9
  u32 AE_TT_2nd_TRIANGLE:       1;  //bit10
  u32 AE_TT_2nd_RESERVED_0:      1;  //bit11
  u32 AE_TT_2nd_RESERVED_1:      1;  //bit12
  u32 AE_TT_2nd_RESERVED_2:      1;  //bit13
  u32 AE_TT_2nd_RESERVED_3:      1;  //bit14
  u32 AE_TT_AUD_HW_NOT_RESET:      1;  //bit15

} AUDIO_TT_CTRL_SWITCH_T;
typedef union _AUDIO_TT_CTRL_SWITCH_UNION_T
{
  AUDIO_TT_CTRL_SWITCH_T ttBitCtrl;
  u32 dwWordCtrl;
} AUDIO_TT_CTRL_SWITCH_UNION_T;

typedef union _AUDIO_TT_FRONT_SWITCH_UNION_T
{
  AUDIO_TT_FRONT_SWITCH_T ttBitFront;
  u32 dwWordFront;
} AUDIO_TT_FRONT_SWITCH_UNION_T;

typedef union _AUDIO_TT_REAR_SWITCH_UNION_T
{
  AUDIO_TT_REAR_SWITCH_T ttBitRear;
  u32 dwWordRear;
} AUDIO_TT_REAR_SWITCH_UNION_T;

typedef struct _AUD_PTS_QUEUE_INFO_
{
    uintptr_t u4QueueSA;
    u32 u4QueueSize;
    uintptr_t u4ReadPtr;
    uintptr_t u4WritePtr;
    uintptr_t u4LastPtsAddr;
    u32 u4LastPtsValue;
    u8  u1AfifoLoop;
    u8  u1SkipPTSCnt;
    u8  u1SkipTempCnt;
    bool   fgFirstPTS;
}AUD_PTS_QUEUE_INFO_T;




#ifdef __cplusplus
extern "C"
{
#endif

// *********************************************************************
// Export API
// *********************************************************************
extern s32 AudioModuleInit(void);
extern void  AudDrvInit(void);
extern void  AudGpsMix_DrvInit(void);
extern void vAudSyncCtrlInfoInit(u8 u1DecId);

extern void AudDrvThreadInit(void);
extern bool AudDrvSetCmd(u8 ucDecId, AUD_DRV_CMD_T eCmd);
extern s32 AudDrvSetDecType(u8 ucDecId,  AUD_DRV_STREAM_FROM_T eStreamFrom,  const AUD_DRV_FMT_INFO_T * prDecType);
extern void AudDrvSetAvSynMode(u8 ucDecId, AV_SYNC_MODE_T eSynMode);
extern void AudDrvGetNfy(u8 u1DecId, AUD_DRV_NFY_INFO_T * prAudNfyInfo);
extern AUD_DRV_STATE_T AudDrvGetState(u8 u1DecId);
extern DECODER_STATE_T AudDrvGetDecState(u8 u1DecId);
extern void vAudUpdateLastPts(u64 u8Pts,u8 u1DecId);

// *********************************************************************
// Export API From Audiodecoder.c
// *********************************************************************
extern s32   AudSetMwCtrl(u8 u1DecId, AUD_DEC_CTRL_T  eAudCtrl);
extern bool  AudSetFormat(u8 u1DecId, AUD_DRV_FMT_INFO_T * prFormatInfo);
extern void  AudSetDecInfo(u8 u1DecId, AUD_DEC_AUD_INFO_T * pv_set_info);
extern bool  AudSetMwCodecInfo(u8 u1DecId, AUD_INFO_T *prInfo);
extern void  AudSetApeSeekInfo(u8 devId, APE_SEEKINFO_INFO_T * pApeSeekInfo);
extern void  AudBackupDecInfo(u8 u1DecId,AUD_DRV_AUD_INFO_T* pv_set_info);

extern void AudSetFrnVolGain(AUD_DEC_VOLUME_GAIN_INFO_T * prChlVolGain, AUD_VOL_POLICY_T eType);
extern void AudGetFrnVolGain(AUD_DEC_VOLUME_GAIN_INFO_T * prChannelVolGain);
extern void AudSetRearVolGain(AUD_DEC_REAR_VOLUME_GAIN_INFO_T * pRearChVol);
extern void AudGetRearVolGain(AUD_DEC_REAR_VOLUME_GAIN_INFO_T * prChannelVolGain);
extern void AudSetFrnVolume(AUD_DEC_VOLUME_INFO_T * prChlVol);
extern void AudGetFrnVolume(AUD_DEC_VOLUME_INFO_T * prChannelVol);
extern void AudSetRearVolume(AUD_DEC_REAR_VOLUME_INFO_T * pRearChVol);
extern void AudSetSrcVolGain(AUD_SRC_VOL_CTL *);
extern void AudGetSrcVolGain(AUD_SRC_VOL_CTL * prSrcVol, u32* pGetVolume);

extern void vAdspEnableASRC(u8 u1DecID,bool fgAsrcEnable);
extern bool fgAdspSetFrontAoutMediaType(AUD_OUT_MEDIA_TYPE_T ptAudMediaOutType);
extern bool fgAdspSetRearAoutMediaType(AUD_OUT_MEDIA_TYPE_T ptAudMediaOutType);
extern bool fgAdspSetMediaType(AUD_MEDIA_TYPE ptAudMediaType);
extern bool fgAdspGetMediaTypeStatus(AUD_MEDIA_TYPE *ptAudMediaType);

extern bool vAdspGetFrontAoutStatus(void);
extern bool vAdspGetRearAoutStatus(void);
extern u8 u1AdspGetFrontAoutType(void);
extern u8 u1AdspGetRearAoutType(void);
extern void AdspMediaSemaInit(void);

#if CONFIG_AUD_DECONLY_EN
extern bool fgAudDeconlySetOnOff(AUD_DECONLY_CTRL_T eDeconlyCtrl);
extern bool fgAudDeconlyGetBuff(AUD_DECONLY_GET_BUF *pGetBuf);
#endif

extern void vAdspSetFeatureInfo(AUD_DEC_FEATURE_INFO_T eModFeatureInfo);
extern void vAdspSetModBManagementInfo(AUD_DEC_MODULE_BMANAGEMENT_CHANNEL_INFO_T eModBManagementInfo);
extern void vAudCodecSet_Bass_Management_Mode(AUD_DEC_BASS_MANAGEMENT_MODE_T t_BM_mode);
extern void vAudCodecSet_DTS_DRC(u16 u2DecId, u16 u2Value);

extern void AudSetCliCmd(AUD_DEC_CLI_TYPE eAudCli,u32 arg1,u32 arg2,
                         u32 arg3,u32 arg4,const s8 **pfilename);
extern void AudSetMute(AUD_DEC1_MUTE_CTRL_T eMuteType);
extern void AudSetDec1Mute(AUD_DEC1_MUTE_CTRL_T eDec1MuteCtrl);
extern bool AudDrvIsDecPlay(u8 u1DecId);
extern void AudSetDvdMixCfg(AUDIO_SAMPLING_T eSmpRate);
extern void AudSetDetectVolThr(AUD_THRESHOLD_T rAudThrshld);
extern void AudGetSpectrumInfo(AUD_DEC_SPECTRUM_INFO_T * ptAudSpectrumInfo);
extern bool AudGetSpectrumData(u8* pAddr, u32 u4Size, u32 u4scaleMode);
extern void AudSetSpkCfg(u8 u1DecId,AUD_DEC_SPEAKER_LAYOUT_T rSpeakerLayout);
extern void AudSetDrc(u8 u1DecId, AUD_DEC_DRC_T eDrc);
extern void AudSetSpdif(AUD_DEC_SPDIF_TYPE_T eSpdif);
extern void AudSetDiversityInfo(AUD_DEC_DIV_TYPE_T eDiversityType, u8 u1_Setting);
extern void AudSetFuncOption(AUD_FUNC_OPTION_T *pvSetting);

//LPCM 2008.1.1 Jimmy temp solution for Legacy
extern void vAudCodecSet_LPCMPara(const AUD_DRV_PCM_SETTING_T *ptPcmSetting);

//HDMI_IN PCM
extern void vAudCodecSet_HDMIInPCMPara(const AUD_DRV_PCM_SETTING_T *ptHDMIInPcmSetting);

extern s32 AudSetDecPlaySpeed(u8 u1DecId, AUD_DEC_PB_SPEED_TYPE_T tSpeed);
extern s32 AudSetDspAsrcBypass(u8 u1DecId, bool fgVal);


/* interface for avsync for wince by mtk40292*/
extern void vAudDrvIf_DisableAVSync(u8 u1DecId);
extern void vAudDrvIf_SetTargetPTS(u8 u1DecId,u64 u8FirstPTS);
extern void vAudDrvIf_GetCurrentPTS(u8 u1DecId,u64* u8FirstPTS);
extern void vAudDrvIf_GetLatestPTS(u8 u1DecId,u32* u4PTSHi,u32* u4PTSLo);

void vAudDrvIf_SwitchAout(u32 dwParam);

// MISC
extern void AudShowDspStatus(void);
extern void AudShowConfig(void);
extern void AudDispStates(void);
extern void AudDispUopHistory(void);
extern void AudDispIECRegisters(void);
extern void vAudDrvIf_DspStopDone(u8 u1DecId);

extern s32 i4AudEsm_Notify_Play(u16 u2ADRV_Comp_Id);
extern s32 i4AudEsm_Notify_Stop(u16 u2ADRV_Comp_Id);

extern void vDspPowerOff (void);

extern void vAudCodecSet_SACD_Input_Info(AUD_DEC_AUD_INFO_T *tInputInfo);
extern void vAudSetSacdInputChannelNum(u32 u4_sacd_input_channel_num);
extern void vAudCodecSet_CDDA_Info(AUD_DEC_AUD_INFO_T  *ptDecInfo);
extern void vAudInSetEmphasisFlag(bool fgEmphasis);
extern void vAudCodecSet_DLNA_Info(void);
extern void vAudCodecDLNAInit(void);
extern void vAudCodecSet_SACD_Output_Info(AUD_DEC_SACD_OUTPUT_T *tInputInfo);

extern s32 i4AUD_CertStart(u8 u1Mode, u8 u1OutCh);
extern s32 i4AUD_CertStop(void);

extern void vAudTestToneSetType(AUD_DEC_TEST_TONE_TYPE_T eType,AUD_DEC_TESTTONE_OUT eTTOut);
extern void vAudTestToneSetChannel(AUD_DEC_LS_T eChannel,AUD_DEC_TESTTONE_OUT eTTOut);
extern void vAudTestToneSwitch(AUD_DEC_TESTTONE_ONOFF fgTTONOFF,AUD_DEC_TESTTONE_OUT eTTOut);
extern void vAudSetDec4Info(AUD_DEC4_INFO_T* ptAudDec4Inf);
extern void vAudLRMixing(AUD_DEC_LRMIX_OUTPUT_T t_lrmix_mode);
extern void vAudGetOutputVol(AUD_OUTPUT_VOL *prChVol);
extern void vAudSetGlobalBoosterGain(s32 i4Gain);
extern void AUD_GetPbInfo(u8 u1DecId, PBINF_A *ptAudPbInfo);
extern void AudSetDecContext(AUD_OUT_MEDIA_TYPE_T eType, AUD_DRV_CONTEXT *prContext, AUD_CFG_ID eAOut);
extern void AudSetDecMediaContext(AUD_MEDIA_TYPE eType, AUD_DRV_CONTEXT *prContext);
extern bool fgAudGetDecStatus(AUD_DEC_ID_T eDecId, AUD_DRV_CONTEXT *prContext);
extern bool AudAllocDecResource(AUD_DRV_CONTEXT *prContext, u8 u1Out, AUD_OUT_MEDIA_TYPE_T eType);
extern void AudReleaseDecResource(AUD_DRV_CONTEXT *prContext);
extern bool AudSetDecPlayBackInfo(AUD_DRV_CONTEXT *prContext, AUD_DEC_AUDIO_PB_INFO_T* prPbInfo);
//miracast API
void   vAudDrvIf_SetMiracastOnOff(AUD_MIRACAST_CTRL_T eMiracastCtrl);
void   vMira_SetAdjustParam(AUD_MIRACAST_PARAM_T rMiraParam);
void   vMira_SetHighThreshold(s64 i8Threshold);
s64  i8Mira_GetHighThreshold(void);
void   vMira_SetLowThreshold(s64 i8Threshold);
s64  i8Mira_GetLowThreshold(void);
void   vMira_SetApllScale(s16 i2Scale);
s16  i2Mira_GetApllScale(void);
void   vMira_SetSleepTime(u16 u2Time);
u16 u2Mira_GetSleepTime(void);
void   vMira_SetDspPtsUpdate(bool fgPtsSet);


                         /* __cplusplus */
#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif /* _AUD_DRV_H_ */

