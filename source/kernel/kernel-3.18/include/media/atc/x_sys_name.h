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

#ifndef _X_SYS_NAME_H_
#define _X_SYS_NAME_H_


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#ifndef __ARM2__
#include "u_common.h"
#endif

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/* The maximum name length (excluding zero terminating character) */
/* will only be 16 characters.                                    */
#define SYS_NAME_LEN  16
#define SYS_IDX_MAX   10

/* System names for the compression / decomprssion engines. */
#define SN_CL_ZIP                     "cl_Zip"
#define SN_CL_ATSC_HUFFMAN_PRG_TITLE  "cl_AtscHmTitle"
#define SN_CL_ATSC_HUFFMAN_PRG_DESC   "cl_AtscHmDesc"
#define SN_CL_SCTE_HUFFMAN_PRG_TITLE  "cl_ScteHmTitle"
#define SN_CL_SCTE_HUFFMAN_PRG_DESC   "cl_ScteHmDesc"

/* System path names to store / retrieve NWL's / SVL's / TSL's / FL's / CFG's */
#define SN_NWL_0         "fs_Nwl_0"
#define SN_NWL_1         "fs_Nwl_1"
#define SN_NWL_2         "fs_Nwl_2"
#define SN_NWL_3         "fs_Nwl_3"
#define SN_NWL_4         "fs_Nwl_4"
#define SN_NWL_5         "fs_Nwl_5"
#define SN_NWL_6         "fs_Nwl_6"
#define SN_NWL_7         "fs_Nwl_7"
#define SN_NWL_8         "fs_Nwl_8"
#define SN_NWL_9         "fs_Nwl_9"
#define SN_NWL_WITH_IDX  "fs_Nwl_%d"

#define SN_MAIN_NWL  SN_NWL_0

#define SN_SVL_0         "fs_Svl_0"
#define SN_SVL_1         "fs_Svl_1"
#define SN_SVL_2         "fs_Svl_2"
#define SN_SVL_3         "fs_Svl_3"
#define SN_SVL_4         "fs_Svl_4"
#define SN_SVL_5         "fs_Svl_5"
#define SN_SVL_6         "fs_Svl_6"
#define SN_SVL_7         "fs_Svl_7"
#define SN_SVL_8         "fs_Svl_8"
#define SN_SVL_9         "fs_Svl_9"
#define SN_SVL_WITH_IDX  "fs_Svl_%d"

#define SN_MAIN_SVL  SN_SVL_0

#define SN_TSL_0         "fs_Tsl_0"
#define SN_TSL_1         "fs_Tsl_1"
#define SN_TSL_2         "fs_Tsl_2"
#define SN_TSL_3         "fs_Tsl_3"
#define SN_TSL_4         "fs_Tsl_4"
#define SN_TSL_5         "fs_Tsl_5"
#define SN_TSL_6         "fs_Tsl_6"
#define SN_TSL_7         "fs_Tsl_7"
#define SN_TSL_8         "fs_Tsl_8"
#define SN_TSL_9         "fs_Tsl_9"
#define SN_TSL_WITH_IDX  "fs_Tsl_%d"

#define SN_MAIN_TSL  SN_TSL_0

#define SN_FL_0         "fs_Fl_0"
#define SN_FL_1         "fs_Fl_1"
#define SN_FL_2         "fs_Fl_2"
#define SN_FL_3         "fs_Fl_3"
#define SN_FL_4         "fs_Fl_4"
#define SN_FL_5         "fs_Fl_5"
#define SN_FL_6         "fs_Fl_6"
#define SN_FL_7         "fs_Fl_7"
#define SN_FL_8         "fs_Fl_8"
#define SN_FL_9         "fs_Fl_9"
#define SN_FL_WITH_IDX  "fs_Fl_%d"

#define SN_MAIN_FL  SN_FL_0

/* System path names to store / retrieve all NWL's / SVL's / TSL's / FL's / CFG's */
/* from a single database / file.                                                 */
#define SN_SYS_CFG_0         "fs_SysCfg_0"
#define SN_SYS_CFG_1         "fs_SysCfg_1"
#define SN_SYS_CFG_2         "fs_SysCfg_2"
#define SN_SYS_CFG_3         "fs_SysCfg_3"
#define SN_SYS_CFG_4         "fs_SysCfg_4"
#define SN_SYS_CFG_5         "fs_SysCfg_5"
#define SN_SYS_CFG_6         "fs_SysCfg_6"
#define SN_SYS_CFG_7         "fs_SysCfg_7"
#define SN_SYS_CFG_8         "fs_SysCfg_8"
#define SN_SYS_CFG_9         "fs_SysCfg_9"
#define SN_SYS_CFG_WITH_IDX  "fs_SysCfg_%d"

#define SN_MAIN_SYS_CFG  SN_SYS_CFG_0

#define SN_RRC_0            "fs_Rrc_0"
#define SN_RRC_1            "fs_Rrc_1"
#define SN_RRC_2            "fs_Rrc_2"
#define SN_RRC_3            "fs_Rrc_3"
#define SN_RRC_4            "fs_Rrc_4"
#define SN_RRC_5            "fs_Rrc_5"
#define SN_RRC_6            "fs_Rrc_6"
#define SN_RRC_7            "fs_Rrc_7"
#define SN_RRC_8            "fs_Rrc_8"
#define SN_RRC_9            "fs_Rrc_9"
#define SN_RRC_WITH_IDX     "fs_Rrc_%d"

#define SN_MAIN_RRC   SN_RRC_0


/* System names for presentation group names. */
#define SN_PRES_MAIN_DISPLAY  "snk_MainDisp"
#define SN_PRES_AUX_DISPLAY   "snk_AuxDisp"
#define SN_PRES_SUB_DISPLAY   "snk_SubDisp"
#define SN_PRES_THIRD_DISPLAY "snk_3rdDisp"

/* System names for mix sound group names. */
#define SN_AUD_MIXSOUND_0     "snk_AudMixSnd_0"
#define SN_AUD_MIXSOUND_1     "snk_AudMixSnd_1"
#define SN_AUD_MIXSOUND_2     "snk_AudMixSnd_2"
#define SN_AUD_MIXSOUND_3     "snk_AudMixSnd_3"
#define SN_AUD_MIXSOUND_4     "snk_AudMixSnd_4"
#define SN_AUD_MIXSOUND_5     "snk_AudMixSnd_5"
#define SN_AUD_MIXSOUND_6     "snk_AudMixSnd_6"
#define SN_AUD_MIXSOUND_7     "snk_AudMixSnd_7"


/* System names for tuner source groups names. */
#define SN_TUNER_GRP_0         "src_TunerGrp_0"
#define SN_TUNER_GRP_1         "src_TunerGrp_1"
#define SN_TUNER_GRP_2         "src_TunerGrp_2"
#define SN_TUNER_GRP_3         "src_TunerGrp_3"
#define SN_TUNER_GRP_4         "src_TunerGrp_4"
#define SN_TUNER_GRP_5         "src_TunerGrp_5"
#define SN_TUNER_GRP_6         "src_TunerGrp_6"
#define SN_TUNER_GRP_7         "src_TunerGrp_7"
#define SN_TUNER_GRP_8         "src_TunerGrp_8"
#define SN_TUNER_GRP_9         "src_TunerGrp_9"
#define SN_TUNER_GRP_WITH_IDX  "src_TunerGrp_%d"

#define SN_MAIN_TUNER_GRP  SN_TUNER_GRP_0


/* Path for raw file system devices. */
#define SN_DEV_PATH  "/dev"


/* Font names */
#define SN_FONT_DEFAULT          "fnt_Default"
#define SN_FONT_DEFAULT_BIG      "fnt_DefaultBig"
#define SN_FONT_MONO_SP_SERF     "fnt_MonoSpSerf"
#define SN_FONT_PROP_SP_SERF     "fnt_PropSpSerf"
#define SN_FONT_MONO_SP_WO_SERF  "fnt_MonoSpWoSerf"
#define SN_FONT_PROP_SP_WO_SERF  "fnt_PropSpWoSerf"
#define SN_FONT_CASUAL           "fnt_Casual"
#define SN_FONT_CURSIVE          "fnt_Cursive"
#define SN_FONT_SMALL_CAPITALS   "fnt_SmallCapital"

#define SN_FONT_VERY_SMALL       "fnt_Small"



/* Individual component / device names */
/* Tuner devices. */
#define SN_TUNER_SAT_DIG_0            "tuner_sat_dig_0"
#define SN_TUNER_SAT_DIG_1            "tuner_sat_dig_1"
#define SN_TUNER_SAT_DIG_2            "tuner_sat_dig_2"
#define SN_TUNER_SAT_DIG_3            "tuner_sat_dig_3"
#define SN_TUNER_SAT_DIG_4            "tuner_sat_dig_4"
#define SN_TUNER_SAT_DIG_5            "tuner_sat_dig_5"
#define SN_TUNER_SAT_DIG_6            "tuner_sat_dig_6"
#define SN_TUNER_SAT_DIG_7            "tuner_sat_dig_7"
#define SN_TUNER_SAT_DIG_8            "tuner_sat_dig_8"
#define SN_TUNER_SAT_DIG_9            "tuner_sat_dig_9"
#define SN_TUNER_SAT_DIG_WITH_IDX     "tuner_sat_dig_%d"

#define SN_MAIN_TUNER_SAT_DIG  SN_TUNER_SAT_DIG_0

#define SN_TUNER_CAB_DIG_0            "tuner_cab_dig_0"
#define SN_TUNER_CAB_DIG_1            "tuner_cab_dig_1"
#define SN_TUNER_CAB_DIG_2            "tuner_cab_dig_2"
#define SN_TUNER_CAB_DIG_3            "tuner_cab_dig_3"
#define SN_TUNER_CAB_DIG_4            "tuner_cab_dig_4"
#define SN_TUNER_CAB_DIG_5            "tuner_cab_dig_5"
#define SN_TUNER_CAB_DIG_6            "tuner_cab_dig_6"
#define SN_TUNER_CAB_DIG_7            "tuner_cab_dig_7"
#define SN_TUNER_CAB_DIG_8            "tuner_cab_dig_8"
#define SN_TUNER_CAB_DIG_9            "tuner_cab_dig_9"
#define SN_TUNER_CAB_DIG_WITH_IDX     "tuner_cab_dig_%d"

#define SN_MAIN_TUNER_CAB_DIG  SN_TUNER_CAB_DIG_0

#define SN_TUNER_OOB_CAB_RX_0         "tuner_oob_c_rx_0"
#define SN_TUNER_OOB_CAB_RX_1         "tuner_oob_c_rx_1"
#define SN_TUNER_OOB_CAB_RX_2         "tuner_oob_c_rx_2"
#define SN_TUNER_OOB_CAB_RX_3         "tuner_oob_c_rx_3"
#define SN_TUNER_OOB_CAB_RX_4         "tuner_oob_c_rx_4"
#define SN_TUNER_OOB_CAB_RX_5         "tuner_oob_c_rx_5"
#define SN_TUNER_OOB_CAB_RX_6         "tuner_oob_c_rx_6"
#define SN_TUNER_OOB_CAB_RX_7         "tuner_oob_c_rx_7"
#define SN_TUNER_OOB_CAB_RX_8         "tuner_oob_c_rx_8"
#define SN_TUNER_OOB_CAB_RX_9         "tuner_oob_c_rx_9"
#define SN_TUNER_OOB_CAB_RX_WITH_IDX  "tuner_oob_c_rx_%d"

#define SN_MAIN_TUNER_OOB_CAB_RX  SN_TUNER_OOB_CAB_RX_0

#define SN_TUNER_OOB_CAB_TX_0         "tuner_oob_c_tx_0"
#define SN_TUNER_OOB_CAB_TX_1         "tuner_oob_c_tx_1"
#define SN_TUNER_OOB_CAB_TX_2         "tuner_oob_c_tx_2"
#define SN_TUNER_OOB_CAB_TX_3         "tuner_oob_c_tx_3"
#define SN_TUNER_OOB_CAB_TX_4         "tuner_oob_c_tx_4"
#define SN_TUNER_OOB_CAB_TX_5         "tuner_oob_c_tx_5"
#define SN_TUNER_OOB_CAB_TX_6         "tuner_oob_c_tx_6"
#define SN_TUNER_OOB_CAB_TX_7         "tuner_oob_c_tx_7"
#define SN_TUNER_OOB_CAB_TX_8         "tuner_oob_c_tx_8"
#define SN_TUNER_OOB_CAB_TX_9         "tuner_oob_c_tx_9"
#define SN_TUNER_OOB_CAB_TX_WITH_IDX  "tuner_oob_c_tx_%d"

#define SN_MAIN_TUNER_OOB_CAB_TX  SN_TUNER_OOB_CAB_TX_0

#define SN_TUNER_TER_DIG_0            "tuner_ter_dig_0"
#define SN_TUNER_TER_DIG_1            "tuner_ter_dig_1"
#define SN_TUNER_TER_DIG_2            "tuner_ter_dig_2"
#define SN_TUNER_TER_DIG_3            "tuner_ter_dig_3"
#define SN_TUNER_TER_DIG_4            "tuner_ter_dig_4"
#define SN_TUNER_TER_DIG_5            "tuner_ter_dig_5"
#define SN_TUNER_TER_DIG_6            "tuner_ter_dig_6"
#define SN_TUNER_TER_DIG_7            "tuner_ter_dig_7"
#define SN_TUNER_TER_DIG_8            "tuner_ter_dig_8"
#define SN_TUNER_TER_DIG_9            "tuner_ter_dig_9"
#define SN_TUNER_TER_DIG_WITH_IDX     "tuner_ter_dig_%d"

#define SN_MAIN_TUNER_TER_DIG  SN_TUNER_TER_DIG_0

/* AV-Connector input's. Since a device may have more than */
/* 10 input devices we allow for 2 digit number at the end */
/* of the string.                                          */
#define SN_AVC_INP_00        "avc_inp_00"
#define SN_AVC_INP_01        "avc_inp_01"
#define SN_AVC_INP_02        "avc_inp_02"
#define SN_AVC_INP_03        "avc_inp_03"
#define SN_AVC_INP_04        "avc_inp_04"
#define SN_AVC_INP_05        "avc_inp_05"
#define SN_AVC_INP_06        "avc_inp_06"
#define SN_AVC_INP_07        "avc_inp_07"
#define SN_AVC_INP_08        "avc_inp_08"
#define SN_AVC_INP_09        "avc_inp_09"
#define SN_AVC_INP_10        "avc_inp_10"
#define SN_AVC_INP_11        "avc_inp_11"
#define SN_AVC_INP_12        "avc_inp_12"
#define SN_AVC_INP_13        "avc_inp_13"
#define SN_AVC_INP_14        "avc_inp_14"
#define SN_AVC_INP_15        "avc_inp_15"
#define SN_AVC_INP_16        "avc_inp_16"
#define SN_AVC_INP_17        "avc_inp_17"
#define SN_AVC_INP_18        "avc_inp_18"
#define SN_AVC_INP_19        "avc_inp_19"
#define SN_AVC_INP_20        "avc_inp_20"
#define SN_AVC_INP_21        "avc_inp_21"
#define SN_AVC_INP_22        "avc_inp_22"
#define SN_AVC_INP_23        "avc_inp_23"
#define SN_AVC_INP_24        "avc_inp_24"
#define SN_AVC_INP_25        "avc_inp_25"
#define SN_AVC_INP_26        "avc_inp_26"
#define SN_AVC_INP_27        "avc_inp_27"
#define SN_AVC_INP_28        "avc_inp_28"
#define SN_AVC_INP_29        "avc_inp_29"
#define SN_AVC_INP_WITH_IDX  "avc_inp_%d"

/* Section filter demuxes */
#define SN_DEMUX_TS_SECTION_MEMORY  NULL
#define SN_DEMUX_SECTION_FILTER     NULL

/* Presentation devices */
#define SN_PLA_MXR                  NULL
#define SN_JPG_DEC                  NULL

/* Non-volatile memory devices. */
#define SN_EEPROM_0                 "eeprom_0"
#define SN_EEPROM_1                 "eeprom_1"
#define SN_EEPROM_2                 "eeprom_2"
#define SN_EEPROM_3                 "eeprom_3"
#define SN_EEPROM_4                 "eeprom_4"
#define SN_EEPROM_5                 "eeprom_5"
#define SN_EEPROM_6                 "eeprom_6"
#define SN_EEPROM_7                 "eeprom_7"
#define SN_EEPROM_8                 "eeprom_8"
#define SN_EEPROM_9                 "eeprom_9"
#define SN_EEPROM_WITH_IDX          "eeprom_%d"

#define SN_NOR_FLASH_0              "nor_0"
#define SN_NOR_FLASH_1              "nor_1"
#define SN_NOR_FLASH_2              "nor_2"
#define SN_NOR_FLASH_3              "nor_3"
#define SN_NOR_FLASH_4              "nor_4"
#define SN_NOR_FLASH_5              "nor_5"
#define SN_NOR_FLASH_6              "nor_6"
#define SN_NOR_FLASH_7              "nor_7"
#define SN_NOR_FLASH_8              "nor_8"
#define SN_NOR_FLASH_9              "nor_9"
#define SN_NOR_FLASH_WITH_IDX       "nor_%d"

#define SN_NAND_FLASH_0             "nand_0"
#define SN_NAND_FLASH_1             "nand_1"
#define SN_NAND_FLASH_2             "nand_2"
#define SN_NAND_FLASH_3             "nand_3"
#define SN_NAND_FLASH_4             "nand_4"
#define SN_NAND_FLASH_5             "nand_5"
#define SN_NAND_FLASH_6             "nand_6"
#define SN_NAND_FLASH_7             "nand_7"
#define SN_NAND_FLASH_8             "nand_8"
#define SN_NAND_FLASH_9             "nand_9"
#define SN_NAND_FLASH_WITH_IDX      "nand_%d"

#define SN_MEM_CARD_0               "ms_0"
#define SN_MEM_CARD_1               "ms_1"
#define SN_MEM_CARD_2               "ms_2"
#define SN_MEM_CARD_3               "ms_3"
#define SN_MEM_CARD_4               "ms_4"
#define SN_MEM_CARD_5               "ms_5"
#define SN_MEM_CARD_6               "ms_6"
#define SN_MEM_CARD_7               "ms_7"
#define SN_MEM_CARD_8               "ms_8"
#define SN_MEM_CARD_9               "ms_9"
#define SN_MEM_CARD_WITH_IDX        "ms_%d"

#define SN_USB_MASS_STORAGE_0	"usb_mass_0"
#define SN_USB_MASS_STORAGE_1	"usb_mass_1"
#define SN_USB_MASS_STORAGE_2	"usb_mass_2"
#define SN_USB_MASS_STORAGE_3	"usb_mass_3"
#define SN_USB_MASS_STORAGE_4	"usb_mass_4"
#define SN_USB_MASS_STORAGE_5	"usb_mass_5"
#define SN_USB_MASS_STORAGE_6	"usb_mass_6"
#define SN_USB_MASS_STORAGE_7	"usb_mass_7"
#define SN_USB_MASS_STORAGE_WITH_IDX	"usb_mass_%d"

#define SN_USB_PTP_MTP_0	"usb_ptpmtp_0"
#define SN_USB_PTP_MTP_1	"usb_ptpmtp_1"
#define SN_USB_PTP_MTP_2	"usb_ptpmtp_2"
#define SN_USB_PTP_MTP_3	"usb_ptpmtp_3"
#define SN_USB_PTP_MTP_4	"usb_ptpmtp_4"
#define SN_USB_PTP_MTP_5	"usb_ptpmtp_5"
#define SN_USB_PTP_MTP_6	"usb_ptpmtp_6"
#define SN_USB_PTP_MTP_7	"usb_ptpmtp_7"
#define SN_USB_PTP_MTP_WITH_IDX	"usb_ptpmtp_%d"

#define SN_USB_HID_0	"usb_hid_0"
#define SN_USB_HID_1	"usb_hid_1"
#define SN_USB_HID_2	"usb_hid_2"
#define SN_USB_HID_WITH_IDX "usb_hid_%d"
#define SN_USB_IPOD_WITH_IDX "usb_ipod_%d"
#define SN_USB_AUD_WITH_IDX  "usb_aud_%d"


#define SN_SCSI_HDD_0       "sda_0"
#define SN_SCSI_HDD_1       "sda_1"
#define SN_SCSI_HDD_2       "sda_2"
#define SN_SCSI_HDD_3       "sda_3"
#define SN_SCSI_HDD_4       "sda_4"
#define SN_SCSI_HDD_5       "sda_5"
#define SN_SCSI_HDD_6       "sda_6"
#define SN_SCSI_HDD_7       "sda_7"

/* Communication devices */
#define SN_COM_RS_232_DBG_PORT      "dbg_rs_232"
#define SN_COM_RS_232_ATV_MNGR_PORT "atv_rs_232"

/* Indicator & ront panel displays */
#define SN_IND_POWER                NULL
#define SN_IND_PLAYBACK             NULL
#define SN_IND_RECORD               NULL
#define SN_IND_FORWARD              NULL
#define SN_IND_REWIND               NULL
#define SN_IND_PAUSE                NULL
#define SN_IND_MAIL                 NULL
#define SN_IND_REMINDER             NULL

#define SN_FP_DISPLAY               NULL

/* IRRC devices */
#define SN_IRRC                     NULL

/* OSD plane devices */
#define SN_OSD_PL_GRAPHIC           "osd_graphic"
#define SN_OSD_PL_IMAGE             "osd_image"

/* POD devices */
#define SN_POD                      NULL

/* Crypto devices */
#define SN_CRYPTO_RANDOM_NUM        NULL
#define SN_CRYPTO_SHA_1             NULL
#define SN_CRYPTO_DFAST             NULL
#define SN_CRYPTO_RSA               NULL
#define SN_CRYPTO_DIFFIE_HELLMAN    NULL
#define SN_CRYPTO_3DES              NULL

#define SN_GCPU  "gcpu"
#define SN_CPSA  "cps_agent"
#define SN_KM    "km"
#define SN_AM    "am"

#define SN_VENC "venc"


    
#endif /* _X_SYS_NAME_H_ */
