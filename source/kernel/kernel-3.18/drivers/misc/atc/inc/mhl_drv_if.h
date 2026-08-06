#ifndef _MHL_DRV_IF_H_
#define _MHL_DRV_IF_H_

/*****************************************************************************
 *
 *  HDMI/MHL Control Code
 *
 *****************************************************************************/
#define IOCTL_MHL_INIT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_CONFIG \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_START \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_STOP \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x3, METHOD_BUFFERED, FILE_ANY_ACCESS)   // 0x2200c

#define IOCTL_MHL_GET_DEVICE_TYPE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x4, METHOD_BUFFERED, FILE_ANY_ACCESS)   // 0x22010

#define IOCTL_MHL_GET_CLOCK_STABLE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x5, METHOD_BUFFERED, FILE_ANY_ACCESS)   // 0x22014

#define IOCTL_MHL_GET_HSYNC_STABLE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_GET_VIDEO_INFO \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_SEND_RCPKEY \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x8, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_MHL_GET_ADDR \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x9, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_GET_PHONE_ORIENTATION \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x10, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_GET_SINGAL_STATUS \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x11, METHOD_BUFFERED, FILE_ANY_ACCESS)
	
//reserved
#define IOCTL_MHL_SET_REG_EMU \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x20, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_GET_REG_EMU \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x21, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_DUMP_REG_EMU \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x22, METHOD_BUFFERED, FILE_ANY_ACCESS)



#define IOCTL_MHL_PROG_HDCP \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x23, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_IS_SUPPORT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x24, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_GET_PHONE_VIDEORECT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x25, METHOD_BUFFERED, FILE_ANY_ACCESS)

//audio driver ioctl ID

#define IOCTL_MHL_AUD_GET_INFO \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x30, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MHL_AUD_SET_INFO \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x31, METHOD_BUFFERED, FILE_ANY_ACCESS)



/*****************************************************************************
 *  global define
 *****************************************************************************/
#define EVENT_NAME_MAX_LENGTH   (20)





/*****************************************************************************
 *
 *  RCP code define
 *
 *****************************************************************************/

//rcp code define
#define KEY_GROUP_DIGIT         ((UINT32) 0x00010000)
#define KEY_GROUP_ALPHA         ((UINT32) 0x00020000)
#define KEY_GROUP_CURSOR        ((UINT32) 0x00030000)
#define KEY_GROUP_SEL_CTRL      ((UINT32) 0x00040000)
#define KEY_GROUP_PRG_CTRL      ((UINT32) 0x00050000)
#define KEY_GROUP_AUD_CTRL      ((UINT32) 0x00060000)
#define KEY_GROUP_TTX_CTRL      ((UINT32) 0x00070000)
#define KEY_GROUP_FCT_CTRL      ((UINT32) 0x00080000)
#define KEY_GROUP_STRM_CTRL     ((UINT32) 0x00090000)
#define KEY_GROUP_DVD_CTRL      ((UINT32) 0x000a0000)
#define KEY_GROUP_USER_DEF      ((UINT32) 0x000b0000)
#define KEY_GROUP_MAX           ((UINT32) 0x000c0000)

#define BTN_DIGIT_0             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '0')))
#define BTN_DIGIT_1             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '1')))
#define BTN_DIGIT_2             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '2')))
#define BTN_DIGIT_3             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '3')))
#define BTN_DIGIT_4             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '4')))
#define BTN_DIGIT_5             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '5')))
#define BTN_DIGIT_6             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '6')))
#define BTN_DIGIT_7             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '7')))
#define BTN_DIGIT_8             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '8')))
#define BTN_DIGIT_9             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '9')))
#define BTN_DIGIT_PLUS_5        ((UINT32)(KEY_GROUP_DIGIT | 0x0000f000))
#define BTN_DIGIT_PLUS_10       ((UINT32)(KEY_GROUP_DIGIT | 0x0000f001))
#define BTN_DIGIT_PLUS_20       ((UINT32)(KEY_GROUP_DIGIT | 0x0000f002))
#define BTN_DIGIT_PLUS_100      ((UINT32)(KEY_GROUP_DIGIT | 0x0000f003))
#define BTN_DIGIT_DOT           ((UINT32)(KEY_GROUP_DIGIT | 0x0000f004))

#define BTN_CURSOR_LEFT         ((UINT32)(KEY_GROUP_CURSOR | 0x0000f000))
#define BTN_CURSOR_RIGHT        ((UINT32)(KEY_GROUP_CURSOR | 0x0000f001))
#define BTN_CURSOR_UP           ((UINT32)(KEY_GROUP_CURSOR | 0x0000f002))
#define BTN_CURSOR_DOWN         ((UINT32)(KEY_GROUP_CURSOR | 0x0000f003))

/*#define BTN_SELECT              ((UINT32)(KEY_GROUP_SEL_CTRL | 0x0000f000))*/
#define BTN_EXIT                ((UINT32)(KEY_GROUP_SEL_CTRL | 0x0000f001))
#define BTN_CLEAR               ((UINT32)(KEY_GROUP_SEL_CTRL | 0x0000f002))

//-----------------------------------------------------------------------


/*****************************************************************************
 *
 *  Video Info
 *
 *****************************************************************************/


#define MHL_EVT_DEVICE  "mhl_evt_dev"
#define MHL_EVT_TIMING  "mhl_evt_timing"

//MHL driver config data
typedef struct
{
    TCHAR szPowerEvent[EVENT_NAME_MAX_LENGTH];
    TCHAR szTimingEvent[EVENT_NAME_MAX_LENGTH];
}MHL_DRV_CONFIG_T;



// scan info
typedef enum {
    SCANINFO_NODATA = 0x0,
    SCANINFO_OVER = 0x1,
    SCANINFO_UNDER = 0x2
} SCAN_INFO_E;

//picture aspect ratio
typedef enum {
    PAR_NODATA = 0x0,
    PAR_4_3 = 0x1,
    PAR_16_9 = 0x2
} PICTURE_ASPECT_RATIO_E;

//active format aspect ratio
typedef enum {
    AFAR_SAME = 0x8,
    AFAR_4_3 = 0x9,
    AFAR_16_9 = 0xa,
    AFAR_14_9 = 0xb,
} ACTIVE_FORMAT_ASPECT_RATIO_E;

//extended colorimetry
typedef enum {
    EC_YCC601 = 0x0,
    EC_YCC709 = 0x1,
} EXTENDED_COLORIMETRY_E;

//rgb color range
typedef enum {
    RQR_DEFAULT = 0x0,
    RQR_LIMIT_RANGE = 0x1,
    RQR_FULL_RANGE = 0x2
} RGB_QUANTIZATIO_RANGE_E;


typedef struct
{
    BOOL bDviMode;     // is dvi or hdmi mode
    UINT32 u4TimingID; // timing ID in CEA861-D
    BOOL bInterlaced;  // is interlaced
    UINT32 u4Width;    // active video width
    UINT32 u4Height;   //active video hegith
    UINT32 u4Hfp;   // Hsync front porch
    UINT32 u4Hbp;   // Hsync back porch
    UINT32 u4Hpw;   // Hsync pulse width
    UINT32 u4Vfp;   // Vsync front porch
    UINT32 u4Vbp;   // Vsync back porch
    UINT32 u4Vpw;   // Vsync pulse width
    UINT32 u4VFreq;
    BOOL bHPol;     // Hsync polarity
    BOOL bVPol;     // Vsync polarity
    BOOL bUVSwap;     // U/V swap
    SCAN_INFO_E eScanInfo;
    PICTURE_ASPECT_RATIO_E ePicAspectRatio;
    ACTIVE_FORMAT_ASPECT_RATIO_E eActiveRation;
    EXTENDED_COLORIMETRY_E eExtenedColorimetry;
    RGB_QUANTIZATIO_RANGE_E eRgbRange;
}MHL_VIDEO_INFO_T;

//device type
typedef enum {
    DEVICE_NULL = 0x0,
    DEVICE_HDMI = 0x1,
    DEVICE_MHL = 0x2
} MHL_DEVICE_TYPE_T;

//-----------------------------------------------------------------------



enum {
    RX_DEVICE_TYPE_NONE,
    RX_DEVICE_TYPE_HDMI,
    RX_DEVICE_TYPE_MHL,
    RX_DEVICE_TYPE_END,
};

enum {
    RX_VDO_SYNC_OFF = 0,
    RX_VDO_SYNC_STABLE,
    RX_VDO_MODE_CHANGE,
    RX_VDO_END,
};

enum {
    RX_MISC_HDCP_START = 0,
    RX_MISC_HDCP_AUTH,
    RX_MISC_HDCP_FAIL,
    RX_MISC_EDID_START,
    RX_MISC_EDID_SUCC,
    RX_MISC_EDID_FAIL,
    RX_MISC_END,
};

enum {
    RX_ERROR_NONE = 0,
    RX_ERROR_HAS_OPEN,
    RX_ERROR_THREAD_CREATE_FAIL,
    RX_ERROR_REG_ISR_FAIL,
    RX_ERROR_SEND_CMD_FAIL,
    RX_ERROR_RCV_CMD_FAIL,
    RX_ERROR_HDCP_FAIL,
    RX_ERROR_EDID_FAIL,
    RX_ERROR_HPD_FAIL,
    RX_ERROR_END,
};


typedef struct _HDCP_KEY_ST {
    unsigned int size;
    char *buf;
} HDCP_KEY_ST;

typedef struct _MHL_WCH_BUF_ADDR {
    unsigned int u4YPhyAddr;
    unsigned int u4CPhyAddr;
} MHL_WCH_BUF_ADDR;

#ifdef __KERNEL__
typedef enum{
    HDMI_SIG_NONE,
    HDMI_SIG_READY,
    HDMI_SIG_LOST,
    HDMI_SIG_CHANGE_START,
    HDMI_SIG_CHANGE_DONE,
    HDMI_SIG_STABLE,
    HDMI_SIG_CONNECTING
} HDMI_SIG_STATE_T;

typedef struct {
	HDMI_SIG_STATE_T signal_state;
	u32			arg;
} HDMI_SIG_INFORMATION;

typedef enum
{
    HDMI_AUD_IDLE,
    HDMI_AUD_STABLE,
} HDMI_AUD_STATE_T;

typedef void (*atc_hdmi_isr_t)(void *arg);
extern int atc_hdmi_register_isr(atc_hdmi_isr_t isr, void *arg);
extern int atc_hdmi_unregister_isr(atc_hdmi_isr_t isr, void *arg);
extern long MHL_IOControl(DWORD cmd, UCHAR *pInBuffer, DWORD inSize, UCHAR *pOutBuffer, DWORD outSize);
#endif //__kernel__

#endif

