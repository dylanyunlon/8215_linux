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

#ifndef _X_RM_DEV_TYPES_H_
#define _X_RM_DEV_TYPES_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "x_common.h"
#include "x_rm.h"


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/* Component types */
#define DRVT_UNKNOWN  ((DRV_TYPE_T)  0)

#define DRVT_TYPE_RM1  ((DRV_TYPE_T) 1)
#define DRVT_TYPE_RM2  ((DRV_TYPE_T) 2)

#define DRVT_TUNER_SAT_DIG         ((DRV_TYPE_T)  16)
#define DRVT_TUNER_CAB_DIG         ((DRV_TYPE_T)  17)
#define DRVT_TUNER_TER_DIG         ((DRV_TYPE_T)  18)
#define DRVT_TUNER_SAT_ANA         ((DRV_TYPE_T)  19)
#define DRVT_TUNER_CAB_ANA         ((DRV_TYPE_T)  20)
#define DRVT_TUNER_TER_ANA         ((DRV_TYPE_T)  21)
#define DRVT_TUNER_CAB_DIG_OOB_TX  ((DRV_TYPE_T)  22)
#define DRVT_TUNER_CAB_DIG_OOB_RX  ((DRV_TYPE_T)  23)
#define DRVT_TUNER_SAT_ANA_SCART_OUT  ((DRV_TYPE_T)  24)
#define DRVT_TUNER_CAB_ANA_SCART_OUT  ((DRV_TYPE_T)  25)
#define DRVT_TUNER_TER_ANA_SCART_OUT  ((DRV_TYPE_T)  26)

#define DRVT_DCC                         ((DRV_TYPE_T)  32)
#define DRVT_DEMUX_TS_PES_PACKET         ((DRV_TYPE_T)  33)
#define DRVT_DEMUX_TS_PES_PACKET_MEMORY  ((DRV_TYPE_T)  34)
#define DRVT_DEMUX_TS_PCR                ((DRV_TYPE_T)  35)
#define DRVT_DEMUX_TS_SECTION_MEMORY     ((DRV_TYPE_T)  36)
#define DRVT_DEMUX_TS_TS_PACKET          ((DRV_TYPE_T)  37)
#define DRVT_DEMUX_TS_TS_PACKET_MEMORY   ((DRV_TYPE_T)  38)
#define DRVT_DEMUX_PS_PES_PACKET         ((DRV_TYPE_T)  39)
#define DRVT_DEMUX_PS_PES_PACKET_MEMORY  ((DRV_TYPE_T)  40)
#define DRVT_DEMUX_SECTION_FILTER        ((DRV_TYPE_T)  41)

#define DRVT_BROADCAST_CA       ((DRV_TYPE_T)  48)
#define DRVT_BROADCAST_CI       ((DRV_TYPE_T)  49)
#define DRVT_PLAYBACK_SEC_MNGR  ((DRV_TYPE_T)  50)

#define DRVT_PLAYBACK_BUFFER    ((DRV_TYPE_T)  51)
#define DRVT_SYNCCTRL           ((DRV_TYPE_T)  52)
#define DRVT_AUDIN_BUFFER    ((DRV_TYPE_T)  53)
#define DRVT_IPODIN_BUFFER    ((DRV_TYPE_T)  54)
#define DRVT_PARTY_MODULE     ((DRV_TYPE_T)  55)

#define DRVT_VID_RESZ ((DRV_TYPE_T)  62)
#define DRVT_VID_NR     ((DRV_TYPE_T)  63)
#define DRVT_VID_DEC    ((DRV_TYPE_T)  64)
#define DRVT_VID_PLANE  ((DRV_TYPE_T)  65)
#define DRVT_PLA_MXR    ((DRV_TYPE_T)  66)
#define DRVT_TV_ENC     ((DRV_TYPE_T)  67)
#define DRVT_AUD_DEC    ((DRV_TYPE_T)  68)
#define DRVT_PCR_DEC    ((DRV_TYPE_T)  69)
#define DRVT_JPG_DEC    ((DRV_TYPE_T)  70)

#define DRVT_ESE        ((DRV_TYPE_T)  71)

#define DRVT_BMP_DEC                        ((DRV_TYPE_T)  72)
#define DRVT_GIF_DEC                        ((DRV_TYPE_T)  73)
#define DRVT_PNG_DEC                        ((DRV_TYPE_T)  74)
#define DRVT_MNG_DEC                        ((DRV_TYPE_T)  75)
#define DRVT_RLE_DEC                        ((DRV_TYPE_T)  76)
#define DRVT_BDPGIG_DEC                     ((DRV_TYPE_T)  77)
#define DRVT_HDDVDSPU_DEC                   ((DRV_TYPE_T)  78)
#define DRVT_DVDSPU_DEC                     ((DRV_TYPE_T)  79)
#define DRVT_AUX_JPG_DEC                    ((DRV_TYPE_T)  81)
#define DRVT_VID_IN                         ((DRV_TYPE_T)  82)
#define DRVT_RTC                            ((DRV_TYPE_T)  80)

#define DRVT_HW_IDE     ((DRV_TYPE_T)  85)
#define DRVT_HW_FCI     ((DRV_TYPE_T)  86)
#define DRVT_HW_USB     ((DRV_TYPE_T)  87)
#define DRVT_HW_1394    ((DRV_TYPE_T)  88)
#define DRVT_HW_NAND    ((DRV_TYPE_T)  89)
#define DRVT_HW_NOR     ((DRV_TYPE_T)  90)
#define DRVT_HW_EEPROM  ((DRV_TYPE_T)  91)
#define DRVT_HW_FDM           ((DRV_TYPE_T)  92)

#define DRVT_EEPROM           ((DRV_TYPE_T)  96)
#define DRVT_NOR_FLASH        ((DRV_TYPE_T)  97)
#define DRVT_NAND_FLASH       ((DRV_TYPE_T)  98)
#define DRVT_MEM_CARD         ((DRV_TYPE_T)  99)
#define DRVT_HARD_DISK        ((DRV_TYPE_T) 100)
#define DRVT_USB_MASS_STORAGE ((DRV_TYPE_T) 101)
#define DRVT_OPTICAL_DISC     ((DRV_TYPE_T) 102)
#define DRVT_OPTICAL_DRIVE    ((DRV_TYPE_T) 103)
#define DRVT_MEM_CARD_READER  ((DRV_TYPE_T) 104)
#define DRVT_HUB              ((DRV_TYPE_T) 105)
#define DRVT_USB_PTP_MTP      ((DRV_TYPE_T) 106)
#define DRVT_USB_HID          ((DRV_TYPE_T) 107)
#define DRVT_USB_IPOD         ((DRV_TYPE_T) 108)
#define DRVT_USB_IPOD_STORAGE ((DRV_TYPE_T) 109)
#define DRVT_USB_AUD          ((DRV_TYPE_T) 110)



#define DRVT_COM_RS_232  ((DRV_TYPE_T)  112)

#define DRVT_IND_POWER     ((DRV_TYPE_T)  128)
#define DRVT_IND_PLAYBACK  ((DRV_TYPE_T)  129)
#define DRVT_IND_RECORD    ((DRV_TYPE_T)  130)
#define DRVT_IND_FORWARD   ((DRV_TYPE_T)  131)
#define DRVT_IND_REWIND    ((DRV_TYPE_T)  132)
#define DRVT_IND_PAUSE     ((DRV_TYPE_T)  133)
#define DRVT_IND_MAIL      ((DRV_TYPE_T)  134)
#define DRVT_IND_REMINDER  ((DRV_TYPE_T)  135)

#define DRVT_FP_DISPLAY  ((DRV_TYPE_T)  144)

#define DRVT_IRRC  ((DRV_TYPE_T)  160)
#define DRVT_KEYBOARD         ((DRV_TYPE_T) 161)

#define DRVT_OSD_PLANE  ((DRV_TYPE_T)  176)

#define DRVT_GPU  ((DRV_TYPE_T)  192)
#define DRVT_VG         ((DRV_TYPE_T)  193)

#define DRVT_POD  ((DRV_TYPE_T)  208)

#define DRVT_CRYPTO_RANDOM_NUM      ((DRV_TYPE_T)  224)
#define DRVT_CRYPTO_SHA_1           ((DRV_TYPE_T)  225)
#define DRVT_CRYPTO_DFAST           ((DRV_TYPE_T)  226)
#define DRVT_CRYPTO_RSA             ((DRV_TYPE_T)  227)
#define DRVT_CRYPTO_DIFFIE_HELLMAN  ((DRV_TYPE_T)  228)
#define DRVT_CRYPTO_3DES            ((DRV_TYPE_T)  229)

#define DRVT_DESCRAMBLER            ((DRV_TYPE_T)  240)

#define DRVT_AVC_COMP_VIDEO  ((DRV_TYPE_T) 256)
#define DRVT_AVC_S_VIDEO     ((DRV_TYPE_T) 257)
#define DRVT_AVC_Y_PB_PR     ((DRV_TYPE_T) 258)
#define DRVT_AVC_VGA         ((DRV_TYPE_T) 259)
#define DRVT_AVC_SCART       ((DRV_TYPE_T) 260)
#define DRVT_AVC_SCART_OUT   ((DRV_TYPE_T) 261)
#define DRVT_AVC_DVI         ((DRV_TYPE_T) 262)
#define DRVT_AVC_HDMI        ((DRV_TYPE_T) 263)
#define DRVT_AVC_AUDIO_INP   ((DRV_TYPE_T) 264)
#define DRVT_AVC_SPDIF       ((DRV_TYPE_T) 265)
#define DRVT_AVC_COMBI       ((DRV_TYPE_T) 266)

#define DRVT_TV_DEC     ((DRV_TYPE_T) 272)

#define DRVT_1394_SRC   ((DRV_TYPE_T) 288)
#define DRVT_1394_DEST  ((DRV_TYPE_T) 289)
#define DRVT_PWR_CTRL   ((DRV_TYPE_T) 304)

#define DRVT_NET_DOCSIS_RFI         ((DRV_TYPE_T) 310)
#define DRVT_NET_ETHERNET_802_3     ((DRV_TYPE_T) 311)
#define DRVT_NET_ETHERNET_802_11    ((DRV_TYPE_T) 312)
#define DRVT_NET_USB                ((DRV_TYPE_T) 313)
#define DRVT_NET_SERIAL             ((DRV_TYPE_T) 314)
#define DRVT_NET_LOOPBACK           ((DRV_TYPE_T) 319)

#define DRVT_HDMI_CEC               ((DRV_TYPE_T) 340)

#define DRVT_BUF_AGENT		        ((DRV_TYPE_T) 350)

#define DRVT_LEGACY_SPLITTER        ((DRV_TYPE_T) 360)
#define DRVT_LEGACY_FILTER          ((DRV_TYPE_T) 361)

#define DRVT_GCPU                   ((DRV_TYPE_T) 362)
#define DRVT_CPSA                   ((DRV_TYPE_T) 363)
#define DRVT_KM                     ((DRV_TYPE_T) 364)
#define DRVT_AM                     ((DRV_TYPE_T) 365)
#define DRVT_SACD                   ((DRV_TYPE_T) 366)

#define DRVT_VFD                    ((DRV_TYPE_T) 370)
#define DRVT_AVD                    ((DRV_TYPE_T) 371)

#define DRVT_DIVERSITY              ((DRV_TYPE_T) 375)
#define DRVT_DIVERSITY_NEW              ((DRV_TYPE_T) 376)
#define DRVT_MISC_RM                ((DRV_TYPE_T) 377)


#define DRVT_GPR_BYTE                    ((DRV_TYPE_T) 380)
#define DRVT_GPR_DBWD                    ((DRV_TYPE_T) 381)
#define DRVT_SYS_COND_STATUS      ((DRV_TYPE_T) 382)

#define DRVT_EADEV                  ((DRV_TYPE_T) 390)

#define DRVT_IFCON                  ((DRV_TYPE_T) 391)

#endif /* _X_RM_DEV_TYPES_H */
