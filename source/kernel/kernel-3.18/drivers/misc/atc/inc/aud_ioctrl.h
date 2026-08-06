#ifndef __AUD_IOCTRL_H__
#define __AUD_IOCTRL_H__
#include "x_aud_dec.h"
#include "drv_config.h"

typedef struct _AFIFO_PHY_ADDRESS_T
{
  char* pbWp;
  __u32 u4Len;
  char* pbSA;
  char* pbEA;
}AFIFO_PHY_INFO_T;


typedef struct _AFIFO_VIR_ADDRESS_T
{
  char* pbWp;
  __u32 u4Len;
  char* pbSA;
  char* pbEA;
}AFIFO_VIR_INFO_T;


/*
//for msdk struct
typedef enum
{
    LIN_DRAM_FRONT,
	LIN_BYPASS_REAR
}LIN_MODE;

typedef enum
{
   LIN_ON,
   LIN_OFF
}LIN_CTRL;
*/
typedef enum
{
    AUD_DEC_LIN_FMT_PCM
}AUD_LININ_DEC_FMT_T;

typedef enum
{
    AUD_DEC_LIN_BIG_ENDIAN,
	AUD_DEC_LIN_SAMLL_ENDIAN
}LIN_ENDIAN;

typedef struct
{
   uintptr_t u4LinDataLen;
}AUDIN_DEC_DATA_LEN;


typedef struct
{
		AUD_DEC_FMT_T		 e_audin_fmt;
		AUD_DEC_AUD_TYPE_T	 e_audin_type;
		__u32				 ui4_sample_rate;
		__u32				 ui4_data_rate;
		__u8				 ui1_bit_depth;
		AUD_DEC_PCM_INFO_T	 pcm_info;
		__u16				 ui2_pid;
	    LIN_ENDIAN           e_linendian;
} AUDIN_INFO;


typedef struct
{
    __u32 u4LinSA;
    __u32 u4LinEA;
}AUDIN_SET_ADDR;


#ifdef __linux__
#define AUD_DEV_NAME                    "/dev/adec"
#define WAV_DEV_NAME                    "TO BE FIXED"
#else
#define AUDIN_DEV_NAME                  TEXT("AIN1:") // will be delete.
#define AUD_DEV_NAME                    TEXT("ADE1:")
#define WAV_DEV_NAME                    TEXT("WAV1:")
#endif

#define AUD_IOCTRL_ID_START             0x800
#define AUDIN_IOCTRL_ID_START           0x1000
#define AUDMHL_IOCTRL_ID_START          0x1100
#define AUD_SMIX_IOCTRL_ID_START        0x1200

#define DEFINE_AUD_IOCTRL(ID)                               \
    CTL_CODE(FILE_DEVICE_UNKNOWN, ID, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*****************************************************************
    Audio Decoder IO Control Define
*****************************************************************/

#define IOCTL_AUDIO_SET_FORMAT                              \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x00)

#define IOCTL_AUDIO_SET_AUD_INFO                            \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x01)

#define IOCTL_AUDIO_GET_VOLUME                              \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x02)

#define IOCTL_AUDIO_SET_SRC_VOLUME                            \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x03)

#define IOCTL_AUDIO_CTL                                     \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x04)

#define IOCTL_AUDIO_SEND_AU                                 \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x05)

#define IOCTL_AUDIO_SET_VOLUME                              \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x06)

#define IOCTL_AUDIO_CONNECT_ESM                             \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x07)

#define IOCTL_AUDIO_DISCONNECT_ESM                          \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x08)

#define IOCTL_AUDIO_SET_PLAY_SPEED                          \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x09)

#define IOCTL_AUDIO_GET_AFIFO_INFO_VIRTUAL                  \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x0A)

#define IOCTL_AUDIO_GET_AFIFO_WRITE_POINTER                 \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x0B)

#define IOCTL_AUDIO_GET_READ_DATA_SUM                       \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x0C)

#define IOCTL_AUDIO_SEND_BUFFER                             \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x0D)

#define IOCTL_AUDIO_SEND_ESM_INFO                           \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x0E)

#define IOCTL_AUDIO_SET_BT_SCO                              \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x0F)

#define IOCTL_AUDIO_SEND_END_OF_STREAM                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x10)

#define IOCTL_AUDIO_GET_SPECTRUM                            \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x11)

#define IOCTL_AUDIO_SET_ORIG_SAMPRATE                       \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x12)

#define IOCTL_AUDIO_SET_SE                                  \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x13)

#define IOCTL_AUDIO_SET_AC3DRC                              \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x14)

#define IOCTL_AUDIO_SET_DIVERSITY_INFO                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x15)

#define IOCTL_AUDIO_SET_APE_SEEKINFO                        \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x16)

#define IOCTL_AUDIO_SET_CLI_CMD_INFO                        \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x17)

#define IOCTL_AUDIO_SET_FEATURE                             \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x18)

#define IOCTL_AUDIO_SET_BMANAGEMENT_MODE                    \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x19)

#define IOCTL_AUDIO_SET_BASS_MANAGEMENT_MODE                \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x1A)

#define IOCTL_AUDIO_SET_DTSDRC                              \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x1B)

#define IOCTL_AUDIO_GET_DEC_CONTEXT                         \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x1C)

#define IOCTL_AUDIO_SET_DEC_CONTEXT                         \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x1D)

#define IOCTL_AUDIO_SET_FRONT_MEDIA_TYPE                    \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x1E)

#define IOCTL_AUDIO_SET_REAR_MEDIA_TYPE                     \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x1F)

#define IOCTL_AUDIO_SET_TARGETPTS                           \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x20)

#define IOCTL_AUDIO_GET_CURRENTPTS                          \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x21)

#define IOCTL_AUDIO_SET_AVSYNC_DISABLE                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x22)

#define IOCTL_AUDIO_SET_SPEAKER_LAYOUT                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x23)

#if CONFIG_AUD_DUAL_PRIMARY_ASRC_SUPPORT
#define IOCTL_AUDIO_SET_ASRC_SWITCH                         \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x24)
#endif //#if CONFIG_AUD_DUAL_PRIMARY_ASRC_SUPPORT

#define IOCTL_AUDIO_SWITCH_AOUT                             \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x25)

#define IOCTL_AUDIO_INFO_FROM_DVP                           \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x26)

#define IOCTL_AUDIO_AOUT_CONFIG                             \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x27)

#define IOCTL_AUDIO_SET_MUTE_TYPE                           \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x28)

#define IOCTL_AUDIO_SET_TEST_TONE_TYPE                      \
	DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x29)

#define IOCTL_AUDIO_SET_TEST_TONE_CHANNEL                   \
	DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x2A)

#define IOCTL_AUDIO_SET_TEST_TONE_ONOFF                     \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x2B)

#define IOCTL_AUDIO_SET_DEC4_INFO                           \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x2C)

#define IOCTL_AUDIO_SET_REAR_VOLUME                         \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x2D)

#define IOCTL_AUDIO_SPDIF_ENABLE                            \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x2E)

#define IOCTL_AUDIO_SET_DAC_TYPE                            \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x2F)

#define IOCTL_AUDIO_GET_FRONT_STATUS                        \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x30)

#define IOCTL_AUDIO_GET_REAR_STATUS                         \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x31)

#define IOCTL_AUDIO_SET_LRMIX                               \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x32)

#define IOCTL_AUDIO_FUNC_OPTION_SET                         \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x33)

#define IOCTL_AUDIO_GET_OUTPUT_VOL                          \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x34)

#define IOCTL_AUDIO_GET_PLAYBACK_INFO                       \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x35)

#define IOCTL_AUDIO_SET_REAR_I2S_GROUP                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x36)

#define IOCTL_AUDIO_GET_REAR_VOLUME                         \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x37)

#define IOCTL_AUDIO_SET_THRESHOLD                           \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x38)

#define IOCTL_AUDIO_SET_TYPE_SPDIF                          \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x39)

#define IOCTL_AUDIO_SET_MUTE_DEC1                           \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x3A)

#define IOCTL_AUDIO_SET_CH_DELAY                            \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x3B)

#define IOCTL_AUDIO_SET_MIRACAST_ONOFF                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x3C)

#define IOCTL_AUDIO_GET_LATEST_PTS                          \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x3D)

#define IOCTL_AUDIO_SET_MIRACAST_PARAM                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x3E)

#define IOCTL_AUDIO_GET_FRONT_TYPE                          \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x3F)

#define IOCTL_AUDIO_GET_REAR_TYPE                           \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x40)

#define IOCTL_AUDIO_FEATURE_SUPPORT                         \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x41)

#define IOCTL_AUDIO_SET_ASRC_BYPASS                         \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x42)

#define IOCTL_AUDIO_GET_CODEC_FIFO_INFO                     \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x43)

#define IOCTL_AUDIO_SET_REAR_OUT_MODE                       \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x44)

#define IOCTL_AUDIO_SEND_BUFFER_KERNEL                      \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x45)

#define IOCTL_AUDIO_GET_CODEC_STATUS                        \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x46)

#define IOCTL_AUDIO_CODEC_RESET                             \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x47)

#define IOCTL_AUDIO_DECONLY_ONOFF                             \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x48)

#define IOCTL_AUDIO_DECONLY_GET_BUF                             \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x49)

#define IOCTL_AUDIO_GET_SPECTRUM_SOURCE                         \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x4A)

#define IOCTL_AUDIO_SET_MEDIA_TYPE                     \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x4B)    

#define IOCTL_AUDIO_GET_MEDIA_TYPE_STATUS                     \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x4C)

#define IOCTL_AUDIO_SET_VOL_POLICY               \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x80)

#define IOCTL_AUDIO_GET_SRC_VOLUME                            \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x81)

#define IOCTL_AUDIO_SET_SRC_MUTE                            \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x82)

#define IOCTL_AUDIO_SET_AUDIO_DEC_INFO                   \
    DEFINE_AUD_IOCTRL(AUD_IOCTRL_ID_START + 0x101)

/*****************************************************************
    Audio IN IO Control Define
*****************************************************************/

#define IOCTL_AUDIN_SET_ONOFF                               \
    DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x00)

#define IOCTL_AUDIN_SET_ADDR                                \
    DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x01)

#define IOCTL_AUDIN_GET_DEC_DATALEN                         \
    DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x02)

#define IOCTL_AUDIN_GET_DEC_CFG_INFO                        \
    DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x03)

#define IOCTL_AUDIN_REAR_VOL_GAIN_INFO                      \
    DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x04)

#define IOCTL_AUDIN_FRONT_VOL_GAIN_INFO                     \
    DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x05)

#define IOCTL_AUDIN_COPY_FROM_USER                            \
    DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x06)

#define IOCTL_AUDIN_IIS_CTRL                                 \
    DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x07)

#define IOCTL_AUDIN_INPUT_TYPE                               \
        DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x08)

#define IOCTL_AUDIN_SET_ADCIN_CTRL                           \
        DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x10)

#define IOCTL_AUDIN_SET_IISIN_CTRL                           \
        DEFINE_AUD_IOCTRL(AUDIN_IOCTRL_ID_START + 0x11)

/*****************************************************************
    Audio MHL IO Control Define
*****************************************************************/

#define IOCTL_AUDMHL_CTL                                    \
    DEFINE_AUD_IOCTRL(AUDMHL_IOCTRL_ID_START + 0x00)

#define IOCTL_AUDMHL_GET_INFO                               \
    DEFINE_AUD_IOCTRL(AUDMHL_IOCTRL_ID_START + 0x01)

#define IOCTL_AUDMHL_PARSING_INFO                           \
    DEFINE_AUD_IOCTRL(AUDMHL_IOCTRL_ID_START + 0x02)

#define IOCTL_AUDMHL_RAW_INFO                               \
    DEFINE_AUD_IOCTRL(AUDMHL_IOCTRL_ID_START + 0x03)

#define IOCTL_AUDMHL_GET_AUDMHLBUFFER_INFO                  \
    DEFINE_AUD_IOCTRL(AUDMHL_IOCTRL_ID_START + 0x04)

#define IOCTL_AUDMHL_MHL_SEND_INFO                          \
    DEFINE_AUD_IOCTRL(AUDMHL_IOCTRL_ID_START + 0x05)


/*****************************************************************
    Audio software mix IO Control Define
*****************************************************************/
#define IOCTL_AUD_SMIX_CTL                                    \
    DEFINE_AUD_IOCTRL(AUD_SMIX_IOCTRL_ID_START + 0x00)

#define IOCTL_AUD_SMIX_SEND_BUFFER                            \
    DEFINE_AUD_IOCTRL(AUD_SMIX_IOCTRL_ID_START + 0x01)


#define IOCTL_AUD_SMIX_GET_FIFO_SIZE                          \
    DEFINE_AUD_IOCTRL(AUD_SMIX_IOCTRL_ID_START + 0x02)

#define IOCTL_AUD_SMIX_GET_FIFO_AVAIL                         \
    DEFINE_AUD_IOCTRL(AUD_SMIX_IOCTRL_ID_START + 0x03)



#endif // #ifndef __AUD_IOCTRL_H__
