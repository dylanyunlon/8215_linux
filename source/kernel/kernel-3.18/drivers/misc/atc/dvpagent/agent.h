#ifndef _AGENT_H_
#define _AGENT_H_

#include <linux/types.h>
#include "chip_ver.h"
#include "dvpcomhw.h"
#include "dvp_protocol.h"

#define MAX_PATH    (256)

#define MAX_DVPDATA_LEN                16

/*DVP addr : ReadReg(IO_VA_BASE + 0x38250) << 24*/
#define DVP_MEM_BANK_REG               0x38308

#define RET_OK                         (u32)(0x0)

#define EVT_DVP_DEVICE_TYPE                    0x01  /* USB /MCR /DISC */
#define EVT_DVP_INIT_STATE                     0x02  /* init state,Loading... */
#define EVT_DVP_TRAY_STATUS                    0x03  /* TRAY IN /TRAY OUT */
#define EVT_DVP_DISC_TYPE                      0x04  /* DVD /VCD */
#define EVT_DVP_LANGUAGE                       0x05  /* Chinese/English  */
#define EVT_DVP_DISC_INSERT                    0x06  /* disc insert */

/* File List, 0x30---0x4f */
#define EVT_DVP_FL_TOTAL_COUNT                 0x30  /* total file count */
#define EVT_DVP_FL_FILE_PATH                   0x31  /* :\A\B\C\D  */
/* item info(name, type(dir/mp3/jpg...)) */
#define EVT_DVP_FL_ITEM                        0x32
/* enter sub-dir/to parent dir */
#define EVT_DVP_FL_CHG_DIR                     0x33

#define EVT_DVP_FL_MODULE                      0x35  /* play module update */

/* Playback info, 0x50---0xfe */
#define EVT_DVP_PBC_ADO_BITRATE                0x50  /* mp3 bitrate */
#define EVT_DVP_PBC_ADO_ID3_TXT                0x51  /* mp3 id3 text */
#define EVT_DVP_PBC_ADO_ID3_PIC                0x52  /* mp3 id3 pic  */
#define EVT_DVP_PBC_ADO_LYRICS                 0x53  /* mp3 lyrics  */
#define EVT_DVP_PBC_ADO_EQ                     0x54  /* mp3 eq data  */
/* jpg Right /left /top /bottom  */
#define EVT_DVP_PBC_JPG_ROTATE_STATE           0x55
#define EVT_DVP_PBC_JPG_RESOLUTION             0x56  /* xxx * xxx  */
#define EVT_DVP_PBC_PLAY_STATE                 0x57  /* PAUSE /PLAY/STOP  */
/* CD, current track/total track  */
#define EVT_DVP_PBC_TRACK_NUM                  0x58
#define EVT_DVP_PBC_CUR_TIME                   0x59  /* HH:MM:SS  */
#define EVT_DVP_PBC_TOTAL_TIME                 0x5a  /* HH:MM:SS  */
#define EVT_DVP_PBC_PLAYING_IDX                0x5b  /* playing index  */
#define EVT_DVP_PBC_REPEAT_MODE                0x5c /* Repeat All/File/Folder */
#define EVT_DVP_PBC_SHUFFLE_MODE               0x5d  /* Shuffle On/Off  */
/* DVD, current title / current chapter  */
#define EVT_DVP_PBC_TITLE_CHAPTER              0x5e
/* DVD, title count / chapter count in current title  */
#define EVT_DVP_PBC_TITLE_CHAPTER_NUM          0x5f
/* need send touch X/Y to dvp in dvd menu state  */
#define EVT_DVP_PBC_DVDHILI_STATUS             0x60
#define EVT_DVP_PBC_AUDIO                      0x61  /*channel, Right /Left  */
#define EVT_DVP_PBC_SUBTITLE                   0x62  /* DVD/MP4 subtitle  */
#define EVT_DVP_PBC_ANGLE                      0x63  /* DVD Angle  */
#define EVT_DVP_PBC_PBC                        0x64  /* VCD PBC ON/OFF */
#define EVT_DVP_CMD_RESPONSE                   0x65

#define DVP_CMD_CONNECT                        0x00
#define DVP_CMD_CHG_DEVICE                     0x01  /* Disc   */
#define DVP_CMD_EJECT                          0x02  /* Eject  */
/* file list, 0x20---0x3f */

/* filter(all/audio/video/pic)  */
#define DVP_CMD_CREATE_LIST                    0x20
/* get item by given index */
#define DVP_CMD_FL_GET_ITEM                    0x21
/* click item by given index */
#define DVP_CMD_FL_CLICK_ITEM                  0x22
#define DVP_CMD_FL_UP_LEVEL                    0x23  /* up level  */
/* playback control, 0x40---0x5f */
#define DVP_CMD_PBC_PLAY                       0x40  /* play  */
#define DVP_CMD_PBC_PAUSE                      0x41  /* pause  */
#define DVP_CMD_PBC_STOP                       0x42  /* stop  */
#define DVP_CMD_PBC_FF                         0x43  /* fast forward  */
#define DVP_CMD_PBC_FB                         0x44  /* fast backward   */
#define DVP_CMD_PBC_PREV                       0x45  /* prev  */
#define DVP_CMD_PBC_NEXT                       0x46  /* next  */
#define DVP_CMD_PBC_SEEK                       0x47  /* goto by time   */
/* function control, 0x60---0x7f */
#define DVP_CMD_FUNC_REPEAT                    0x60  /* repeat  */
#define DVP_CMD_FUNC_SHUFFLE                   0x61  /* shuffle   */
#define DVP_CMD_FUNC_AUDIO                     0x62  /* audio  */
#define DVP_CMD_FUNC_ANGLE                     0x63  /* angle  */
#define DVP_CMD_FUNC_SUBTITLE                  0x64  /* subtitle   */
#define DVP_CMD_FUNC_TITLE                     0x65  /* title   */
#define DVP_CMD_FUNC_MENU                      0x66  /* menu  */
#define DVP_CMD_FUNC_PBC                       0x67  /* vcd pbc   */
#define DVP_CMD_FUNC_ROTATE                    0x68  /* jpg rotate   */
#define DVP_CMD_FUNC_DIGEST                    0x69  /* jpg digest  */
/* other, 0x80---0xfd */

/* touch panel x/y coordinate  */
#define DVP_CMD_TP_COORDINATE                  0x80
#define EVT_DVP_ERROR_OCCURED                  0x66

// Audio Video switch
#define AP_CMD_AV_SWITCH                        0xF1
    #define CMD_VIDEO           0X01
    #define CMD_AUDIO           0x02


#define MZ_SUCCESS  0x00000000
#define MZ_FAILURE  0x80000000

/*EVT_DVP_DISC_TYPE: disc type */
#define SV_DVD_VIDEO                   0
#define SV_DVD_AUDIO                   1
#define SV_MINI_DVD                    2
#define SV_VCD3_0                      3
#define SV_VCD2_0                      4
#define SV_VCD1_1                      5
#define SV_VCD6_0                      6
#define SV_VCD6_1                      7
#define SV_SVCD                        8
#define SV_CVD                         9
#define SV_CDDA                        10
#define SV_HDCD                        11
#define SV_DTS_CS                      12
#define SV_CDG                         13
#define SV_PCD                         14
#define SV_FJCD                        15
#define SV_DATA                        16
#define SV_UPG                         17
#define SV_SACD                        18
#define SV_AUDIO_IN                    19   /* set by 8032 only */
#define SV_DVD_VR                      20
#define SV_XVCD                        21
#define SV_XSVCD                       22
#define SV_UNKNOWN                     23


/* EVT_DVP_INIT_STATE: INIT_STATE */
#define SV_LOADING_DISC                0
#define SV_TRAY_OPENING                1
#define SV_TRAY_CLOSING                2
#define SV_TRAY_ERROR                  3
#define SV_NO_DISC                     4
#define SV_UNKNOWN_DISC                5
#define SV_DISC_IDENTIFIED             6
#define SV_DISC_INITED                 7
#define SV_EMD_LOADING                 8
#define SV_NO_USB                      9

/*EVT_DVP_PBC_PLAY_STATE:Play Status */
#define SV_STOP                        1
#define SV_PLAY                        2
#define SV_FF                          3
#define SV_SF                          4
#define SV_FR                          5
#define SV_SR                          6
#define SV_PAUSE                       7
#define SV_STEP                        8
#define SV_STEP_BACK                   9
#define SV_FREEZE                      10
#define SV_STILL                       11
#define SV_SETUP_PAUSE                 12
#define SV_STOP_RESUME                 13
/* note the definition before this can not be changed! */
#define SV_DIGEST9                     14
#define SV_DIGEST4                     15
#define SV_DIGEST                      16
#define SV_PTL_ERR                     17
#define SV_REGION_ERR                  18
#define SV_DISC_ERR                    19
#define SV_SET_TMP_PTL                 20
#define SV_PBC_INIT                    21
#define SV_WAIT_LASTMEM                22
#define SV_PRE_PLAY                    23

#define SV_PRE_NEXT                    30
#define SV_PRE_PREV                    31

#define fgIsDiscPlay(bPbcState)      (((bPbcState > SV_STOP) \
                && (bPbcState < SV_STOP_RESUME)) \
                || (bPbcState == SV_PRE_PLAY))

/*repeat mode */
#define SV_USR_STOP                    1
#define SV_REPEAT_NONE                 2
#define SV_USR_PBC_NORMAL              2
#define SV_REPEAT_ALL                  3
#define SV_REPEAT_TRACK                4
#define SV_REPEAT_LIST                 5
#define SV_REPEAT_TITLE                6
#define SV_REPEAT_CHAPTER              7
#define SV_REPEAT_SET_A                8
#define SV_REPEAT_AB                   9
#define SV_MIC_REP_A                   10
#define SV_MIC_REP_AB                  11
#define SV_REPEAT_LAST_SP              12
#define SV_REPEAT_INSTANT              13
#define SV_REPEAT_ABNONE               14
#define SV_RANDOM                      15
#define SV_SHUFFLE                     16
#define SV_PROGRAM                     17
#define SV_USR_DIGEST                  18
#define SV_SINGLE                      19
#define SV_SCAN                        20
#define SV_REPEAT_FOLDER               21
#define SV_SHUFFLE_FOLDER              22
/*      file type */
#define FTYPE_UNKNOWN                  0xFF
#define FTYPE_AC3                      0x00
#define FTYPE_MP3                      0x01
#define FTYPE_MP2                      0x02
#define FTYPE_MP1                      0x03
#define FTYPE_JPG                      0x05
#define FTYPE_MLP                      0x06
#define FTYPE_WMA                      0x07
#define FTYPE_ASF                      0x08
#define FTYPE_MPG                      0x09
#define FTYPE_DAT                      0x0A
#define FTYPE_VOB                      0x0B
#define FTYPE_AAC                      0x0C
#define FTYPE_DSD                      0x0D
#define FTYPE_CDA                      0x0F
#define FTYPE_DIR                      0x10  /* directory */
#define FTYPE_DTS                      0x11
#define FTYPE_AVI                      0x12
#define FTYPE_TS1                      0x13  /* for test */
#define FTYPE_MP4                      0x14
#define FTYPE_3GP                      0x15
#define FTYPE_TS2                      0x16  /* for test */
#define FTYPE_DST                      0x17
#define FTYPE_OGG                      0x18
#define FTYPE_TS4                      0x19
#define FTYPE_RCV                      0x1A
#define FTYPE_VC1                      0x1B
#define FTYPE_MKV                      0x1C
#define FTYPE_M4A                      0x1D
#define FTYPE_OMA                      0x1E
#define FTYPE_RA                       0x1F
#define FTYPE_PDIR                     0x20  /* parent directory */

/* languae */
#define   MSG_ENGLISH                  0x1
#define   MSG_FRENCH                   0x2
#define   MSG_SPANISH                  0x3
#define   MSG_CHINESE                  0x4
#define   MSG_JAPANESE                 0x5
#define   MSG_KOREAN                   0x6
#define   MSG_GERMAN                   0x7
#define   MSG_PORTUGUESE               0x8
#define   MSG_ITALIAN                  0x9
#define   MSG_THAI                     0xA
#define   MSG_SWEDISH                  0xb
#define   MSG_DANISH                   0xc
#define   MSG_NORWEGIAN                0xd
#define   MSG_FINNISH                  0xe
#define   MSG_DUTCH                    0xf
#define   MSG_ICELANDIC                0x10
#define   MSG_HEBREW                   0x11
#define   MSG_GREEK                    0x12
#define   MSG_CROATIAN                 0x13
#define   MSG_TURKISH                  0x14
#define   MSG_POLISH                   0x15
#define   MSG_HUNGARIAN                0x16
#define   MSG_CZECH                    0x17
#define   MSG_RUSSIAN                  0x18
#define   MSG_INDONESIAN               0x19
#define   MSG_OFF                      0xFF



#define fgIsUsrCtrlMode(bMode) ((bMode >= SV_RANDOM)  \
                        && (bMode <= SV_USR_DIGEST))
#define fgISUsrSeqPlay(bMode)  ((bMode == SV_REPEAT_NONE)  \
                        || (bMode == SV_REPEAT_ABNONE))
#define fgIsRepeatMode(bMode)  ((bMode >= SV_REPEAT_ALL)   \
                        && (bMode <= SV_REPEAT_CHAPTER))
#define fgIsA2BMode(bMode)   ((bMode >= SV_REPEAT_SET_A) \
                        || (bMode < SV_REPEAT_ABNONE))


/* data disc/usb player mode */
#define SV_DDISC_MODULE_NONE           0x0
#define SV_DDISC_MODULE_MP3            0x1
#define SV_DDISC_MODULE_JPG            0x2
#define SV_DDISC_MODULE_MPG            0x3

/* On/Off */
#define SV_ON                          0x0
#define SV_OFF                         0x1

#define SV_FP_DOM                      1
#define SV_VMGM_DOM                    2
#define SV_VTSM_DOM                    3
#define SV_VTSTT_DOM                   4
#define SV_ATSTT_DOM                   5
#define SV_PLAY_LIST                   6
#define SV_SELECT_LIST                 7
#define IsInDvdMenu(bDom)   ((bDom > SV_FP_DOM) && (bDom < SV_VTSTT_DOM))

/*cd ripping state information  */
#define SV_CD_RIP_NONE                 (0)
#define SV_CD_RIP_FULL                 (1)
#define SV_CD_RIP_DONE                 (2)
#define SV_CD_RIP_FAIL                 (3)
#define SV_CD_RIP_ABORT                (4)
#define SV_CD_RIP_EXIST                (5)
#define SV_CD_RIP_WRITING              (6)
#define SV_CD_RIP_DISC_ERROR           (7)
#define SV_CD_RIP_ABORT_OK             (8)
#define SV_CD_RIP_DEV_WRITE_PROTECT    (9)
#define SV_CD_RIP_FOLDER_IS_FULL       (10)
#define SV_CD_RIP_FILE_IS_FULL         (11)
#define SV_CD_RIP_FAT_NOT_SUPPORT      (12)

/*  device ID for device manager (must be kept as unique and starts from 1) */
/*  for USB card slot */
/* sum of USB_SLOT_1 to USB_SLOT_6 */
#define SV_DEVID_USB_SLOT_ALL          0x0f
#define SV_DEVID_USB_SLOT_1            0x00
#define SV_DEVID_USB_SLOT_2            0x01
#define SV_DEVID_USB_SLOT_3            0x02
#define SV_DEVID_USB_SLOT_4            0x03
#define SV_DEVID_USB_SLOT_5            0x04
#define SV_DEVID_USB_SLOT_6            0x05
/*  for MS Card */
#define SV_DEVID_MS_CARD               0x06
/*  for SD Card */
#define SV_DEVID_SD_CARD               0x07
/*  for SM Card */
#define SV_DEVID_SM_CARD               0x08
/*  for CF Card */
#define SV_DEVID_CF_CARD               0x09
/*  for TVD */
#define SV_DEVID_TVD                   0x0A
/*  for DVBT */
#define SV_DEVID_DVBT                  0x0B
/*  for Audio */
#define SV_DEVID_AUDIOIN               0x0C
/*  for CD-DVD ROM */
#define SV_DEVID_CDVDROM               0x0D
/*  for INVALID device */
#define SV_DEVID_INVALID               0xff

/*EVT_DVP_FL_MODULE*/
#define FLMD_NONE                      0x0
#define FLMD_MP3_PLAYER                0x1
#define FLMD_JPEG_DECODER              0x2
#define FLMD_MPEG_PLAYER               0x3

/*CD Rip error code*/
#define ERROR_CD_RIP_PATH_NOT_EXIST    0xF1
#define ERROR_CD_RIP_NOT_ENUGH_SPACE   0xF2

enum E_DVP_PBMODE {
    DVP_PBMODE_REPEAT,
    DVP_PBMODE_SHUFFLE
};

enum E_DVP_IMAGE_VIEWMODE {
    DVP_IMAGE_VIEWMODE_DIGEST
};

#define SV_PBC_OFF                     0
#define SV_PBC_ON                      1
#define SV_PBC_TP_ON                   2
#define SV_PBC_TP_OFF                  3

#define MAX_AP2DVP_LEN                 16

#define FS_RAM_CODE_MAX_FILENAME_BUF_SIZE      82

struct DVP_FILEITEM_INFO_T {
    u8  uType;
    u16 szFileName[FS_RAM_CODE_MAX_FILENAME_BUF_SIZE + 1];
};

struct DVP_DRAM_FL_LIST_ITEM_T {
    u8 uFType;                                     /* File type */
    u8 szName[FS_RAM_CODE_MAX_FILENAME_BUF_SIZE];  /* Filename */
    u8 uFinish;                                    /* 4-byte alignment  */
};

struct MSG_STRUCT_T {
    bool    fgFromPT;
    u8      uDVPData[600];
    u32     u4Len;
};

struct Dvp_Data_Len {
    u32      dwDataLen;
    bool     fgFromPT;
};

struct DVP_Rip_Path {
    u16  *path;
    u32  u4Len;
    u32  u4TrkLen;
};

enum E_DVP_LOG_TYPE {
    DVP_8032_LOG,
    DVP_PT110_LOG
};

struct DVP_LOG_SET {
    enum E_DVP_LOG_TYPE  eLogType;
    u32             fgDVPLog;
};

struct DVP_Dram_Data {
    u32          dwStartAddress;
    void         *pvBufferData;
};

bool DVPAgent_Init(void);
void DVPAgent_Deinit(void);
bool DVPAgent_SetRipPath(struct DVP_Rip_Path *pRipPath);
u32 DVPHost_Write8032(u8 *puData, u32 u4DataLen);
u32 DVPHost_WritePT110(u8 *puData, u32 u4DataLen);
s32 DVPAgent_SendCmd(u8 bInstID, struct AP2DVPCMD_T *prCmd);
bool DVPAgent_CopyLyricsBuf(u8 *puData, u32 u4DataLen);
bool DVPAgent_CopyId3PicBuf(u8 *puData, u32 u4DataLen);
bool DVPAgent_CopyRipData(s8 *puRipBuf, u32 u4Len);
bool DVPAgent_SetDVPLog(struct DVP_LOG_SET *prDVPLog);
bool DVPAgent_SetCodePage(u32 u4CodePage);
bool DVPAgent_WriteData2Dram(struct DVP_Dram_Data *prDramData, u32 u4Size);
bool DVPAgent_ReadDataFromDram(struct DVP_Dram_Data *prDramData, u32 u4Size);
bool DVPAgent_ReadFileInfoFromFsIO(struct DVP_FILEITEM_INFO_T *prFileInfo,
                        u32 u4Size, u32 u4ReadIndex);
void DVPHost_ChangeDeviceIdle(void);
void DVPHost_Deinit(void);

bool DVPAgent_parseAudioID3TXT(struct DVP2APPACKET_T *rPacket);
bool DVPAgent_parseAudioID3PIC(struct DVP2APPACKET_T *rPacket);
bool DVPAgent_parseLyrics(struct DVP2APPACKET_T *rPacket);
bool DVPAgent_parseItemInfo(struct DVP2APPACKET_T *rPacket);
bool DVPAgent_riptrklbalen(struct DVP2APPACKET_T *rPacket);
bool DVPAgent_filePath(struct DVP2APPACKET_T *rPacket);
bool DVPAgent_CurrentfilePath(struct DVP2APPACKET_T *rPacket);
void DVPAgent_getRipAddress(struct DVP2APPACKET_T *rPacket);

void DVPAgent_SendtoMainIoctl(bool fgFromPT, struct DVP2APPACKET_T *rPacket);
void DVPAgent_SaveDrmData(void);

u32 changePhyToVirtualAddress(u32 address);
u32 getDvpBaseVirtualAddress(void);

extern unsigned int MetaZone_WriteBinary(unsigned int u4Idx, const char *pbData, unsigned int u4Size);
extern u32 MetaZone_Flush(int fgSync);

extern u32 DVPFs_GetPathByIndex(u16 u2Index, s8 *pPathBuf, u32 u4BufSize);


#define MAX_CMD_NUM 100

#ifdef DVP_PM_SUPPORT
bool DVPAgent_PowerOff(void);
bool DVPAgent_PowerOn(void);
#endif

#ifdef MM_SUPPORT_DIVXHT31
void loadDrmData();
#endif

#endif

