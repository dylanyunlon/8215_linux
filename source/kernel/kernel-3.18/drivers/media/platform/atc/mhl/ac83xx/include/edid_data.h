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

#ifndef _EDID_DATA_H_
#define _EDID_DATA_H_

/* #if (DRV_SUPPORT_HDMI_RX) */
#include "x_typedef.h"
#include "typedef.h"
#include "mhl_private.h"

/* ============================================================================= */

#define EDID_SUPPORT_PCM_2CH_ONLY 0  /* default 2ch */
#define EDID_SUPPORT_BYPASS_TX_EDID  0  /* Set Tx's EDID to Rx'EDID*/
/* #define EDID_SUPPORT_HD_AUDIO       **** from pinpin chen  default hdmi in not support HD Audio */

#if CONFIG_DRV_CUSTOM_JXF == 1
#define EDID_SUPPORT_NO_DEEP_COLOR_ONLY 0
#define EDID_SUPPORT_PCM_ONLY_2CH 1
#else
#define EDID_SUPPORT_NO_DEEP_COLOR_ONLY 1
#define EDID_SUPPORT_PCM_ONLY_2CH 0
#endif


#if 1/* CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT3363 */
#define INTERAL_MODE       0    /* both EDID use interal EDID  can not support 4Kx2K */
#define TWO_EXT_MODE     1   /* both EDID use extern EEPROM as EDID  can support 4Kx2K */
#define MIX_MODE              2   /* one use interal ram,and the other using EEPROM as EDID, Default HDMI TRx port using EEPROM as EDID can support 4Kx2K */

#define EDID_SUPPORT_QHD  0   /* support 4k2k */

#if ((CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX))
#ifdef DRV_SUPPORT_DUAL_EXIT_EDID_HDMI_RX
#if DRV_SUPPORT_DUAL_EXIT_EDID_HDMI_RX == 1
#define EDID_SUPPORT_MODE   TWO_EXT_MODE
#else
#define EDID_SUPPORT_MODE   INTERAL_MODE
#endif
#else
#define EDID_SUPPORT_MODE   INTERAL_MODE
#endif

/* #elif CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8555 */
/* #define EDID_SUPPORT_MODE   TWO_EXT_MODE */
#else
#define EDID_SUPPORT_MODE   TWO_EXT_MODE
#endif

#endif
#define _PRINT_EDID_
#define _PRINT_EDID_READ_WRITE_

#ifdef  EDID_SUPPORT_HD_AUDIO
#define EDID_DEFAULT_BL1_ADDR_PHY       0x29
#else
#define EDID_DEFAULT_BL1_ADDR_PHY       0x20
#endif

#define HDMI_INPUT_COUNT                2

#define EEPROM_ID                       0xA0
#define EDID_BLOCK_SIZE                 128
#define EDID_ADR_CHECK_SUM              0x7F
#define EDID_BL0_ADR_EXTENSION_NMB      0x7E

#define EDID_BL0_ADR_HEADER             0x00
#define EDID_BL0_LEN_HEADER             8

#define EDID_BL0_ADR_VERSION            0x12
#define EDID_BL0_ADR_REVISION           0x13

#define EDID_MONITOR_NAME_DTD           0xFC
#define EDID_MONITOR_RANGE_DTD          0xFD

#define EDID_BL1_ADR_DTD_OFFSET         0x02
#define EDID_BL0_ADR_DTDs               0x36
#define END_1stPAGE_DESCR_ADDR          0x7E
#if CONFIG_DRV_CUSTOM_JSN

#define EDID_MANUFACTURER_ID    0x4D, 0xD9/* SONY //0x4C, 0x2D   //SAM */


#define EDID_PRODUCT_ID         0x02, 0x9E/* BDV5G_E_G */
#define EDID_PRODUCT_ID1        0x02, 0x2A/* BDV4G H system */
#define EDID_SERIAL_NUMBER      0x01, 0x01, 0x01,0x01       //00000001 */
#define EDID_WEEK               0x05     /* week 40 */
#define EDID_YEAR               0x15    /* 2010 */
#define VENDOR_NAME 'S', 'O', 'N', 'Y', ' ', 'A', 'V', 'S', 'Y', 'S', 'T', 'E', 'M'    /*  "SONY AVAMP " */

#elif CONFIG_DRV_CUSTOM_SFLP

#define EDID_MANUFACTURER_ID    0x41, 0x0C /*  PHL  //0x4C, 0x2D //SAM */
#define EDID_PRODUCT_ID         0x00, 0x00/* 0x01, 0x00  //0001 */
#define EDID_SERIAL_NUMBER      0x00, 0x00, 0x00, 0x00       /* 00000001 */
#define EDID_WEEK               0x00
#define EDID_YEAR               0x15    /* 2011 */
#define VENDOR_NAME 'P', 'H', 'I', 'L', 'I', 'P', 'S', 0x0A, ' ', ' ', ' ', ' ', ' '  /*  "PHILIPS " */

#elif CONFIG_DRV_CUSTOM_JXF

#define EDID_MANUFACTURER_ID     0x41, 0x2F /* PIO  0x41, 0x0C // PHL  //0x4C, 0x2D  //SAM */
#define EDID_PRODUCT_ID     0x00, 0x00/* 0x01, 0x00  //0001 */
#define EDID_SERIAL_NUMBER      0x00, 0x00, 0x00,0x00       /* 00000001 */
#define EDID_WEEK               0x00
#define EDID_YEAR               0x17    /* 2013 */
#define VENDOR_NAME 'B', 'D', '-', 'H', 'T', 'S', 0x0A, ' ', ' ', ' ', ' ', ' ', ' '  /*  "MT85xx-AVR0 " */

#else

#define EDID_MANUFACTURER_ID    0x36, 0x8B /* MTK   0x41, 0x2F //PIO  0x41, 0x0C // PHL  //0x4C, 0x2D    //SAM */
#define EDID_PRODUCT_ID         0x00, 0x00/* 0x01, 0x00  //0001 */
#define EDID_SERIAL_NUMBER      0x00, 0x00, 0x00, 0x00       /* 00000001 */
#define EDID_WEEK               0x00
#define EDID_YEAR               0x15    /* 2011 */
#define VENDOR_NAME 'B', 'D', '-', 'H', 'T', 'S', 0x0A, ' ', ' ', ' ', ' ', ' ', ' '  /*  "MT85xx-AVR0 " */

#endif
#define DEFAULT_MIN_V_HZ            48 /*  Min. Vertical rate (for interlace this refers to field rate), in Hz. */
#define DEFAULT_MAX_V_HZ            62 /*  Max. Vertical rate (for interlace this refers to field rate), in Hz. */
#define DEFAULT_MIN_H_KHZ           23 /*  Min. Horizontal in kHz */
#define DEFAULT_MAX_H_KHZ           47 /*  Max. Horizontal in kHz, */
#define DEFAULT_MAX_PIX_CLK_10MHZ   8 /*  Max. Supported Pixel Clock, in MHz/10, rounded */

#define EDID_VIDEO_BLOCK_LEN        20 /* 17 */
#ifdef EDID_SUPPORT_HD_AUDIO
#define EDID_AUDIO_BLOCK_LEN        28
#if EDID_SUPPORT_PCM_ONLY_2CH == 1
#define EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN       4
#else
#define EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN       22
#endif
#else   /* EDID_SUPPORT_HD_AUDIO */
#define EDID_AUDIO_BLOCK_LEN        16
#if EDID_SUPPORT_PCM_ONLY_2CH == 1
#define EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN       4
#else
#define EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN       13
#endif

#endif  /* EDID_SUPPORT_HD_AUDIO */

#define EDID_SPEAKER_BLOCK_LEN      4
#define EDID_VENDOR_BLOCK_FULL_LEN  22 /*15 */
#define EDID_VENDOR_BLOCK_4K_2K_FULL_LEN    19

#define EDID_VENDOR_BLOCK_LEN       7

#define EDID_VENDOR_BLOCK_MINI_LEN      7  /* jitao.shi@20100921 for Sony mini VSDB include support Ai information */
#define   EDID_COLORMETRY_BLOCK_LEN     4
#define   EDID_VCDB_BLOCK_LEN     3
#define   EDID_DTD_BLOCK_LEN 72 /* '4*18 */

#define AUDIO_TAG           0x20
#define VIDEO_TAG           0x40
#define VENDOR_TAG          0x60
#define SPEAKER_TAG         0x80

#define EEPROM0 0
#define EEPROM1 1
#define EEPROM2 2


#define EDIDDEV0 0
#define EDIDDEV1 1
#define EDIDDEV2 2

#define Printf  printk
#if CONFIG_DRV_CUSTOM_JSN
extern INT32 ifcon_rw_registry(REGISTRY_ID_T e_registry,
			       UINT8 *	pui1Buf,
			       UINT32 ui4_buf_size,
			       BOOL b_read,
			       BOOL b_store);
#endif


typedef struct _PHY_ADDR_PARAMETER_T {
	UINT16    Origin;
	UINT16    Dvd;
	UINT16    Sat;
	UINT16    Tv;
	UINT16    Else;
} PHY_ADDR_PARAMETER_T;


typedef struct _EDID_PARAMETER_T {
	UINT8 PHYLevel;
	UINT8 bBlock0Err;
	UINT8 bBlock1Err;
	UINT8 PHYPoint;
	UINT8 bCopyDone;
	UINT8 bDownDvi;
	UINT8 Number;
} EDID_PARAMETER_T;





void EdidProcessing(void);
void Default_Edid_BL0_BL1_Write(void);
void vReadRxEDID(UINT8 u1EDID);
void vVerifyWEdidBL0(UINT8 u1Index);
void vVerifyREdidBL0(UINT8 u1Index);
void vSetEdidUpdateMode(BOOL fgVideoAndOp, BOOL fgAudioAndOP);
void vCEAAudioDataAndOperation(UINT8 *prData, UINT8 bCEALen, UINT8 *poBlock, UINT8 *poCount);
void vSetEdidPcm2chOnly(UINT8 u12chPcmOnly);
void vShowEdidPhyAddress(void);
void vModifyEdidPa(UINT16 u2PA);
void vRecoverEdidPa(void);
void vShowEditionEdidBlock(void);
void vModifyEditionEdid(UINT8 Addr, UINT8 u1Value);
void vWriteEditionEdidToEEPROM(void);
void vSelectEdidToEdit(UINT8 u1EdidNum, UINT8 u1Block);
void vQuitEdidEdition(void);
void vEDIDCreateBlock1(void);

void vWriteEDIDBlk0(UINT8 u1EDIDNo, UINT8 *poBlock);
void vWriteEDIDBlk1(UINT8 u1EDIDNo, UINT8 *poBlock);

UINT8 bGetCbusEDID(UINT8 uOffset);
extern void ComposeEdidBlock0(UINT8 *pr_rBlock, UINT8 *pr_oBlock);
extern void Default_Edid_BL0_BL1_Write(void);
extern BOOL fgOnlySupportPcm2ch(void);
extern BOOL fgOnlySupportNoDeep(void);

#endif
