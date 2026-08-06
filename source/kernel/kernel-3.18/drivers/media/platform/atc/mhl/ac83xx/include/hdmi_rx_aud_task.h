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

#ifndef _HDMI_RX_AUD_TASK_H_
#define _HDMI_RX_AUD_TASK_H_

#include  "typedef.h"
#include  "x_audin.h"


#define HDMIRX_INTR_STATE0 0x070
#define HDMIRX_AUD_PACKET            ((unsigned)0x1 << 17)   /* Received Audio packet */

#define HDMIRX_INTR_STATE1 0x078
#define HDMIRX_AFIFO_UNDERRUN        \
	((unsigned)0x1 <<  0)   /* Audio FIFO under-run. This interrupt bit is not set for all under-run conditions */
#define HDMIRX_AFIFO_OVERRUN         ((unsigned)0x1 <<  1)   /* Audio FIFO over-run. */
#define HDMIRX_CTS_REUSED            ((unsigned)0x1 <<  2)
#define HDMIRX_CTS_DROPPED           ((unsigned)0x1 <<  3)
#define HDMIRX_TERC4_ERROR           \
	((unsigned)0x1 <<  5)    /* TERC4 error. Set when number of TERC4 errors exceeds threshold */
#define HDMIRX_HDCP_ERROR         ((unsigned)0x1 <<  6)  /* HDCP error. Set when decryption fails. */
#define HDMIRX_AUD_FS_CHG            ((unsigned)0x1 <<  8)   /* Audio FS sample rate changed. Write 1 to clear */
#define HDMIRX_SPDF_LR_ERR           ((unsigned)0x1 << 17)   /* SPDIF left/right error */
#define HDMIRX_AFIFO_PREOVERRUN      ((unsigned)0x1 << 22)   /* Audio FIFO pre over-run */
#define HDMIRX_AFIFO_PREUNDERRUN     ((unsigned)0x1 << 23)   /* Audio FIFO pre under-run */
#define HDMIRX_DSD_PACKET            ((unsigned)0x1 << 24)   /* One Bit Audio(SACD) Packet Received. */
#define HDMIRX_HBR_PACKET            ((unsigned)0x1 << 25)   /* HBR Packet Received. */
#define HDMIRX_AUD_CHSTS_CHG         ((unsigned)0x1 << 28)   /* Audio Channel Status change */

#define HDMIRX_INT_STATUS_CHK  \
	(HDMIRX_AFIFO_UNDERRUN|HDMIRX_AFIFO_OVERRUN|HDMIRX_AUD_FS_CHG)/* |HDMIRX_TERC4_ERROR | HDMIRX_HDCP_ERROR) */
#define HDMIRX_ LK_THRS_SVAL 0x100
#define HDMIRX_AUD_MCLKOUT                ((unsigned)0x3 << 20)   /* Audio output MCLK */
#define HDMIRX_AUD_MCLK_OUT_128FS         ((unsigned)0x0 << 20)
#define HDMIRX_AUD_MCLK_OUT_256FS         ((unsigned)0x1 << 20)
#define HDMIRX_AUD_MCLK_OUT_384FS         ((unsigned)0x2 << 20)
#define HDMIRX_AUD_MCLK_OUT_512FS         ((unsigned)0x3 << 20)
#define HDMIRX_AUD_MCLK_IN                 ((unsigned)0x3 << 22)   /* Audio Input MCLK */
/*Audio MCLK Input Frequency Mode Software Select:
  $00 :  Fm = 128* Fs
  $01 :  Fm = 256 * Fs (default)
  $10 :  Fm = 384 * Fs
  $11 :  Fm = 512 * Fs
  If the pins MCLKIN and MCLKOUT are connected together, then this field must match SWMCLKOUT.*/
#define HDMIRX_AUD_MCLK_IN_128FS         ((unsigned)0x0 << 22)
#define HDMIRX_AUD_MCLK_IN_256FS         ((unsigned)0x1 << 22)
#define HDMIRX_AUD_MCLK_IN_384FS         ((unsigned)0x2 << 22)
#define HDMIRX_AUD_MCLK_IN_512FS         ((unsigned)0x3 << 22)

#define HDMIRX_LK_THRS_SVAL 0x114
#define HDMIRX_AUD_FS_MASK               \
	((unsigned)0xF << 24)   /* Hardware Extracted Sampling Frequency (Fs). */

/*This value is used by the receiver to reconstruct MCLK.
 These bits are extracted from the Channel Status bits 27:24
 in the HDMI audio packets, and represent the Fs rate (in KHz):
 $0000 :  44.1 (default
 $1000 :  88.2
 $1100 :  176.4
 $0010 :  48
 $1010 :  96
 $1110 :  192
 $0011 :  32
 $0001 :  sample frequency not indicated.
 This register shows the same value as AUD_FS in CHST4 at address 0x30 . Values not listed are reserved.*/

#define HDMIRX_I2S_CTRL 0x124

#define HDMIRX_AUD_SD_DELAY                  \
	((unsigned)0x1 << 16)   /*  WS to SD Shift First Bit  0 : First Bit Shift (Philips Spec), 1 : No Shift. */
#define HDMIRX_AUD_SD_DELAY_1B               ((unsigned)0x0 << 16)
#define HDMIRX_AUD_SD_NO_DELAY               ((unsigned)0x1 << 16)
#define HDMIRX_AUD_SD_DIRECT                 \
	((unsigned)0x1 << 17)   /* SD Data Direction (MSB or LSB First) 0/1 :
					Most/Least-Significant Bit (MSB)/(LSB) first. */
#define HDMIRX_AUD_SD_MSB                    ((unsigned)0x0 << 17)
#define HDMIRX_AUD_SD_LSB                    ((unsigned)0x1 << 17)
#define HDMIRX_AUD_SD_JUSTIFY                \
	((unsigned)0x1 << 18)   /* SD Justification:  0 : Data is Left Justified. 1 : Data is Right Justified */
#define HDMIRX_AUD_SD_LJ                     ((unsigned)0x0 << 18)
#define HDMIRX_AUD_SD_RJ                     ((unsigned)0x1 << 18)
#define HDMIRX_AUD_WD_LR_POLARITY            \
	((unsigned)0x1 << 19)    /*  Word Select Left/Right Polarity:
					0/1 : Left polarity when Word Select is LOW/HIGH. */
#define HDMIRX_AUD_WD_LR_LOW                 ((unsigned)0x0 << 19)
#define HDMIRX_AUD_WD_LR_HIGH                ((unsigned)0x1 << 19)
#define HDMIRX_AUD_MSB_EXT                   \
	((unsigned)0x1 << 20)    /*  Most-Significant Bit Sign-Extended: 0 : Enabled, 1 : Disabled. */
#define HDMIRX_AUD_MSB_EXT_EN                ((unsigned)0x0 << 20)
#define HDMIRX_AUD_MSB_EXT_DIS               ((unsigned)0x1 << 20)
#define HDMIRX_AUD_LRCK_WS                   \
	((unsigned)0x1 << 21)    /*  Word Size:  0 : 32 bits ,  1 : 16 bits. */
#define HDMIRX_AUD_LRCK_WS_32                ((unsigned)0x0 << 21)
#define HDMIRX_AUD_LRCK_WS_16                ((unsigned)0x1 << 21)
#define HDMIRX_AUD_CLK_EDGE                  \
	((unsigned)0x1 << 22)    /*  Sample Clock Edge: 0 : rising (positive) ,1 : falling (negative) */
#define HDMIRX_AUD_CLK_RISING                ((unsigned)0x0 << 22)
#define HDMIRX_AUD_CLK_FALLING               ((unsigned)0x1 << 22)
#define HDMIRX_AUD_I2S_VALID                 \
	((unsigned)0x1 << 23)    /*  Send Invalid Data Enable: 0 : Send only valid data (default).1 : Send all data. */
#define HDMIRX_AUD_VALID_CHK                 ((unsigned)0x0 << 23)
#define HDMIRX_AUD_VALID_NEG                 ((unsigned)0x1 << 23)
#define HDMIRX_AUD_I2S_SEL                   \
	((unsigned)0x1 << 24)    /*  PCM Only - I2S Data Pass Select , 0 : Pass all data , 1 : Zero out if RAW */
#define HDMIRX_AUD_PASS                      ((unsigned)0x0 << 24)
#define HDMIRX_AUD_RAW_ZERO                  ((unsigned)0x1 << 24)
#define HDMIRX_AUD_VUCP_MASK                 \
	((unsigned)0x1 << 25)    /*  0 :  send 24-bit serial data. 1 : send 28-bit with VUCP */
#define HDMIRX_AUD_NO_VUCP                   ((unsigned)0x0 << 25)
#define HDMIRX_AUD_WITH_VUCP                 ((unsigned)0x1 << 25)
#define HDMIRX_AUD_MCLK_OUT                  \
	((unsigned)0x1 << 27)    /*  SMCLK Enable: 0 :  = tri-state MCLKOUT. 1 :  = enable MCLKOUT. */
#define HDMIRX_AUD_MCLK_TRI                  ((unsigned)0x0 << 27)
#define HDMIRX_AUD_MCLK_EN                   ((unsigned)0x1 << 27)
#define HDMIRX_AUD_SD_MASK                   ((unsigned)0xF << 28)    /*  SD I2S Output channel control */
#define HDMIRX_AUD_SD0_EN                    ((unsigned)0x1 << 28)
#define HDMIRX_AUD_SD1_EN                    ((unsigned)0x1 << 29)
#define HDMIRX_AUD_SD2_EN                    ((unsigned)0x1 << 30)
#define HDMIRX_AUD_SD3_EN                    ((unsigned)0x1 << 31)

#define HDMIRX_AUDRX_CTRL 0x128
#define HDMIRX_AUD_SD0_MAP                    ((unsigned)0x3 <<  0)  /* SD0 Map */
#define HDMIRX_AUD_SD1_MAP                    ((unsigned)0x3 <<  2)  /* SD1 Map */
#define HDMIRX_AUD_SD2_MAP                    ((unsigned)0x3 <<  4)  /* SD2 Map */
#define HDMIRX_AUD_SD3_MAP                    ((unsigned)0x3 <<  6)  /* SD3 Map */
/*00 : Select stream #0
  01 : Select stream #1
  10 : Select stream #2
  11 : Select stream #3.*/
#define HDMIRX_AUD_SPDF_EN                    \
	((unsigned)0x1 <<  8)  /* S/PDIF Output Enable:
	0 : Disabled (output flat line or zero according to SPMODE). 1 : Enabled. */

#define HDMIRX_AUD_SPDF_MODE                  ((unsigned)0x1 <<  9)

#define HDMIRX_AUD_I2S_MODE                   \
	((unsigned)0x1 << 10)  /*  2S Output Mode
	0 : All I2S outputs are grounded (SD, SCK, WS) (default).
	1 : SCK and WS toggle, SD is on or off depending on the value in I2S_CTRL2[4]. */

#define HDMIRX_AUD_AUDIO_ERR                  \
	((unsigned)0x1 << 11)   /*  Pass S/PDIF Error
	0 : repeat last samples, 1 : pass all audio data regardless of error */
#define HDMIRX_AUD_AUDIO_ERR_RPT              ((unsigned)0x0 << 11)
#define HDMIRX_AUD_AUDIO_ERR_PASS             ((unsigned)0x1 << 11)
#define HDMIRX_AUD_SPDF_ERR                  \
	((unsigned)0x1 << 12)   /*  Pass S/PDIF Error
	0 : repeat good samples, 1 : pass all audio data regardless of error */
#define HDMIRX_AUD_SPDF_ERR_RPT              ((unsigned)0x0 << 12)
#define HDMIRX_AUD_SPDF_ERR_PASS             ((unsigned)0x1 << 12)
#define HDMIRX_AUD_HW_MUTE                   ((unsigned)0x1 << 13)   /*  Hardware Mute Enable */
#define HDMIRX_AUD_HW_MUTE_DIS               ((unsigned)0x0 << 13)
#define HDMIRX_AUD_HW_MUTE_EN                ((unsigned)0x1 << 13)
#define HDMIRX_AUDCH_STAT1_CP                \
	((unsigned)0x01 << 16)  /*  0 : consumer application , 1 : professional application */
#define HDMIRX_AUDCH_STAT1_AUDBIT            \
	((unsigned)0x01 << 17)  /*  Audio/Digital bit :
	0 : audio sample word represents linear PCM samples ,1 : audio sample word used for other purposes */
#define HDMIRX_AUDCH_STAT1_COPY              \
	((unsigned)0x01 << 18)  /*  Copyright information */
#define HDMIRX_AUDCH_STAT1_PREEMP            \
	((unsigned)0x07 << 19)  /*  Pre-empahsis information */
#define HDMIRX_AUDCH_STAT1_MODE              \
	((unsigned)0x03 << 22)  /*  00 Mode 0 for digital audio equipment for consumer use */
#define HDMIRX_AUDCH_STAT2_CATEGORY          \
	((unsigned)0xFF << 24)  /*   Category Code (corresponds to channel status bits 15:8) */

#define HDMIRX_AUD_CHTS0  0x12C
#define HDMIRX_AUD_SOURCE1                   \
	((unsigned)0x0F <<  0)  /*  Source Number (corresponds to channel status bits 19:16) */
#define HDMIRX_AUD_CH_NUM1                   \
	((unsigned)0x0F <<  4)  /*  Channel Number (corresponds to channel status bits 23:20) */
#define HDMIRX_AUD_DIV_INCR                  \
	((unsigned)0xFF <<  8)  /*  Audio out soft mute divider minimum value. DIV_MAX = 1024 - DIV_INCR; */
#define HDMIRX_AUD_CS_OW                     \
	((unsigned)0x01 << 16)  /*  Channel Status Overwrite Enable*/
/*#define HDMIRX_AUD_CS_BIT2                   \
	((unsigned)0x01 << 18)   ****  Channel Status Bit #2 Overwrite Data.
			This bit value is used for CHST1 Bit 2 when OW_CHEN=1. */

#define HDMIRX_AUD_I2S_SWAP                  \
	((unsigned)0x0F << 20)  /*  Swap Left/Right on I2S Channel[3:0],
					0 : No swap ,1 : Swap left and right channels on I2S Channel */

#define HDMIRX_AUD_CS_BIT15_8                ((unsigned)0xFF << 24)

#define HDMIRX_AUD_CHTS1  0x130
#define HDMIRX_AUD_SAMPLE_F                  \
	((unsigned)0x0F <<  0) /*  Sampling Frequency (channel status bits 27:24) */
#define HDMIRX_AUD_ACCURACY                  \
	((unsigned)0x0F <<  4) /*  Clock Accuracy (corresponds to channel status bits 31:28) */
#define HDMIRX_AUD_LENGTH_MAX                \
	((unsigned)0x01 <<  8) /*  Audio Length Max (channel status bit 32)
					0/1 :  = maximum sample word length is 20/24 bits */
#define HDMIRX_AUD_LENGTH_MAX_20            ((unsigned)0x00 <<  8)
#define HDMIRX_AUD_LENGTH_MAX_24            ((unsigned)0x01 <<  8)
#define HDMIRX_AUD_LENGTH                    ((unsigned)0x07 <<  9) /*  Audio Length (channel status bits 35:33) */
#define HDMIRX_AUD_ORG_FS                    \
	((unsigned)0x0F << 12) /*  Original sampling frequency (channel status bits 39:36) */
#define HDMIRX_AUD_CH0_MUTE                  ((unsigned)0x01 << 16)
#define HDMIRX_AUD_CH1_MUTE                  ((unsigned)0x01 << 17)
#define HDMIRX_AUD_CH2_MUTE                  ((unsigned)0x01 << 18)
#define HDMIRX_AUD_CH3_MUTE                  ((unsigned)0x01 << 19)
#define HDMIRX_AUD_I2S_LENGTH                \
	((unsigned)0x0F << 20)  /*  Audio Sample Length Override */
#define HDMIRX_AUD_HDMIMOD_SEL               \
	((unsigned)0x01 << 24)  /*  HDMI_MODE select , 0 : hardware ,1 : software */
#define HDMIRX_AUD_HDMIMOD_HW               ((unsigned)0x00 << 24)
#define HDMIRX_AUD_HDMIMOD_SW               ((unsigned)0x01 << 24)
#define HDMIRX_AUD_HDMIMOD_SOFT_VALUE        ((unsigned)0x1 << 25)  /*  HDMI_MODE software value */

#define HDMIRX_AUDP_STATE 0x134
#define HDMIRX_AUD_PACKET_LAYOUT    ((unsigned)0x1 << 3)

#define HDMIRX_AUDP_INFOFRAME_H 0x180
#define HDMIRX_AUD_INFOFRAME_TYPE        ((unsigned)0xFF <<  0)   /*  AUDIO InfoFrame Type Code. Required 0x84 */
#define HDMIRX_AUD_INFOFRAME_VER         ((unsigned)0xFF <<  8)   /*  AUDIO InfoFrame Version Code. Required 0x01 */
#define HDMIRX_AUD_INFOFRAME_LEN         ((unsigned)0xFF << 16)   /*  AUDIO InfoFrame Length. Required 0x0A */
#define HDMIRX_AUD_INFOFRAME_CHKSUM      ((unsigned)0xFF << 24)   /*  AUDIO InfoFrame Checksum. */

#define HDMIRX_AUDP_INFOFRAME_DB14 0x184
#define HDMIRX_AUD_INFOFRAME_DB1         ((unsigned)0xFF <<  0)   /*  AUDIO InfoFrame Data Bytes1 */
#define HDMIRX_AUD_INFOFRAME_DB2         ((unsigned)0xFF <<  8)   /*  AUDIO InfoFrame Data Bytes2 */
#define HDMIRX_AUD_INFOFRAME_DB3         ((unsigned)0xFF << 16)   /*  AUDIO InfoFrame Data Bytes3 */
#define HDMIRX_AUD_INFOFRAME_DB4         ((unsigned)0xFF << 24)   /*  AUDIO InfoFrame Data Bytes4 */

#define HDMIRX_AUDP_INFOFRAME_DB58 0x188
#define HDMIRX_AUD_INFOFRAME_DB5         ((unsigned)0xFF <<  0)   /*  AUDIO InfoFrame Data Bytes5 */
#define HDMIRX_AUD_INFOFRAME_DB6         ((unsigned)0xFF <<  8)   /*  AUDIO InfoFrame Data Bytes6 */
#define HDMIRX_AUD_INFOFRAME_DB7         ((unsigned)0xFF << 16)   /*  AUDIO InfoFrame Data Bytes7 */
#define HDMIRX_AUD_INFOFRAME_DB8         ((unsigned)0xFF << 24)   /*  AUDIO InfoFrame Data Bytes8 */

#define HDMIRX_AUDP_INFOFRAME_DB910 0x18C
#define HDMIRX_AUD_INFOFRAME_DB9         ((unsigned)0xFF <<  0)   /*  AUDIO InfoFrame Data Bytes9 */
#define HDMIRX_AUD_INFOFRAME_DB10        ((unsigned)0xFF <<  8)   /*  AUDIO InfoFrame Data Bytes10 */

#define AUD_MCLK_128FS  0x0
#define AUD_MCLK_256FS  0x1
#define AUD_MCLK_384FS  0x2
#define AUD_MCLK_512FS  0x3

#define M_Fs 0xF
#define B_Fs_UNKNOW    1
#define B_Fs_44p1KHz    0
#define B_Fs_48KHz  2
#define B_Fs_32KHz  3
#define B_Fs_88p2KHz    8
#define B_Fs_96KHz  0xA
#define B_Fs_176p4KHz   0xC
#define B_Fs_192KHz 0xE
#define B_Fs_768KHz 0x9 /*  1001 */
#define B_Fs_HBR 0x9 /*  1001 */

#define B_CAP_AUDIO_ON  (1<<7)
#define B_CAP_HBR_AUDIO (1<<6)
#define B_CAP_DSD_AUDIO (1<<5)
#define B_LAYOUT        (1<<4)
#define B_MULTICH       (1<<4)
#define B_HBR_BY_SPDIF  (1<<3)
#define B_SPDIF         (1<<2)
#define B_CAP_LPCM      (1<<0)

#define B_CAP_AUDIO_ON_SFT  7
#define B_CAP_HBR_AUDIO_SFT 6
#define B_CAP_DSD_AUDIO_SFT 5
#define B_MULTICH_SFT       4
#define B_HBR_BY_SPDIF_SFT  3
#define B_SPDIF_SFT        2
#define B_CAP_LPCM_SFT   0

#define B_AUDIO_ON    (1<<7)
#define B_HBRAUDIO    (1<<6)
#define B_DSDAUDIO    (1<<5)
#define B_AUDIO_LAYOUT     (1<<4)
#define M_AUDIO_CH         0xF
#define B_AUDIO_SRC_VALID_3 (1<<3)
#define B_AUDIO_SRC_VALID_2 (1<<2)
#define B_AUDIO_SRC_VALID_1 (1<<1)
#define B_AUDIO_SRC_VALID_0 (1<<0)


#define B_AUD_NLPCM (1<<1)

/* #define B_TRI_VIDEOIO (1<<6) */
/* #define B_TRI_VIDEO (1<<5) */
/* #define B_TRI_SPDIF (1<<4) */
#define B_TRI_I2S3 (1<<7)
#define B_TRI_I2S2 (1<<6)
#define B_TRI_I2S1 (1<<5)
#define B_TRI_I2S0 (1<<4)
#define B_MUTE_CH3  (1<<3)
#define B_MUTE_CH2  (1<<2)
#define B_MUTE_CH1  (1<<1)
#define B_MUTE_CH0  (1<<0)

#define B_TRI_ALL  (B_TRI_VIDEOIO|B_TRI_VIDEO|B_TRI_SPDIF|B_TRI_I2S3|B_TRI_I2S2|B_TRI_I2S1|B_TRI_I2S0)
#define B_TRI_AUDIO  (B_TRI_I2S3|B_TRI_I2S2|B_TRI_I2S1|B_TRI_I2S0)

#define B_MUTE_AUDIO  (B_MUTE_CH3|B_MUTE_CH2|B_MUTE_CH1|B_MUTE_CH0)

#define B_TRI_MASK  ~(B_TRI_VIDEOIO|B_TRI_VIDEO|B_TRI_SPDIF|B_TRI_I2S3|B_TRI_I2S2|B_TRI_I2S1|B_TRI_I2S0)
#define B_TRI_I2S  (B_TRI_I2S3|B_TRI_I2S2|B_TRI_I2S1|B_TRI_I2S0)


/*  2008/08/15 added by jj_tseng@chipadvanced */
#define F_AUDIO_ON  (1<<7)
#define F_AUDIO_HBR (1<<6)
#define F_AUDIO_DSD (1<<5)
#define F_AUDIO_NLPCM (1<<4)
#define F_AUDIO_LAYOUT_1 (1<<3)
#define F_AUDIO_LAYOUT_0 (0<<3)

#define T_AUDIO_MASK 0xF0
#define T_AUDIO_OFF 0
#define T_AUDIO_HBR (F_AUDIO_ON|F_AUDIO_HBR)
#define T_AUDIO_DSD (F_AUDIO_ON|F_AUDIO_DSD)
#define T_AUDIO_NLPCM (F_AUDIO_ON|F_AUDIO_NLPCM)
#define T_AUDIO_LPCM (F_AUDIO_ON)

typedef struct {
	BYTE AudioFlag;
	BYTE AudSrcEnable;
	BYTE SampleFreq;
	BYTE ChStat[5];
	Audio_InfoFrame AudInf;/* kenny add */
	RX_REG_AUDIO_CHSTS AudChStat;/* kenny add */
} AUDIO_CAPS;

/*
const CHAR *AStateStr[6] = {
    "ASTATE_AudioOff",
    "ASTATE_RequestAudio",
    "ASTATE_ResetAudio",
    "ASTATE_WaitForReady",
    "ASTATE_AudioOn",
    "ASTATE_Reserved"
};
*/
BOOL GetAudioChannelStatus(RX_REG_AUDIO_CHSTS *RegChannelstatus, UINT8 audio_status);
void SetupAudio(void);
void EnableAudio(void);
void getHDMIRxInputAudio(AUDIO_CAPS *pAudioCaps);
void SetAudioMute(BOOL bMute);
void AssignAudioTimerTimeout(UINT32 TimeOut);
void getHDMIRxInputChStat(AUDIO_CAPS *pAudioCaps);
void SetMUTE(BYTE AndMask, BYTE OrMask);
void  vUpdateHdmiRxAudio(void);
/* ==== Export HDMI Rx Audio interface ===== */
extern void vHDMIRxAudMainTask(void);
extern void SwitchAudioState(Audio_State_Type state);
extern void vGetHdmiRxAudioParameter(HDMI_RX_IN_AUDIO_INFO_T *prAudIf);
extern void vSendHDMIRxAudMuteInt(void);
extern void vSendHDMIRxAudUnMuteInt(void);
extern int ExportAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame);
extern void ExportAudioChannelStatus(RX_REG_AUDIO_CHSTS *RegChannelstatus);
extern void vHDMIRxUnPlugNotifyAud(void);
extern BOOL fgHDMIAudRxBypassMode(void);
extern void vGetInternalHDMIAudChannelStatus(UINT8 *u1AudChStatus);
extern UINT8 GetRxAudioState(void);
extern void vNotifyHDMIRxACPTypeChange(UINT8 u1ACPType);
/* extern UINT32 _u4DebugRxMessageType; */
extern void AudmhlSendAudMsg(UINT32 u4Cmd, BYTE bPri);
extern void ADAC_SpecialMute(BOOL fgEnable);
extern BOOL fgNotifySignal;
extern BOOL MhlSendAudInfo(UINT32 u4Msg);
extern void AudmhlSendAudMsg(UINT32 u4Cmd, BYTE bPri);
extern void ADAC_SpecialMute(BOOL fgEnable);
extern void HalHDMIRxEnableAudPktReceive(void);
extern BOOL fgNotifySignal;

void vHdmiRxAudLoop(void);

#endif

