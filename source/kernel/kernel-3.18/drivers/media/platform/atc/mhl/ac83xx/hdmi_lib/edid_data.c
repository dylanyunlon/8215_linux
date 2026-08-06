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

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/timer.h>
#include <linux/module.h>

#include "x_lint.h"
#include "x_typedef.h"
#include "x_os.h"
#include "x_printf.h"

#include "x_bim.h"
#include "x_assert.h"
#include "drv_av_d.h"
/* #include "drv_hdmi_rx.h" */
#include "x_hal_ic.h"
#include "x_timer.h"
#include "mhl_drv_if.h"

#include "hdmi_rx_ctrl.h"
#include "drv_hdmi_rx.h"

/****************************************************************************
** Local definitions
****************************************************************************/
#if 0/*(DRV_SUPPORT_HDMI_RX)*/
#include "edid_eeprom.h"
#include "edid_data.h"
#include "rx_io.h"




UINT8 const Default_Edid_Block0[128] = {
/*  (00H)      Header */
	0x00,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0x00,

/* (08H-09H)  ID Manufacturer Name _________________________    = EDID_MANUFACTURER_ID
(0AH-0BH)  Product ID Code ______________________________    = EDID_PRODUCT_ID
(0CH-0FH)  Last 5 Digits of Serial Number _______________    = NOT USED
(10H)      Week of Manufacture __________________________    = 0
(11H)      Year of Manufacture __________________________    = 2008 */
	EDID_MANUFACTURER_ID,  EDID_PRODUCT_ID,
	EDID_SERIAL_NUMBER,
	EDID_WEEK,
	EDID_YEAR,

/*  (12H)      EDID version 1.3 */
	0x01,  0x03,

/*           Basic Display Parameters
(14H)      VIDEO INPUT DEFINITION: Digital Signal
(15H)      Maximum Horizontal Image Size ________________    =   undefined
(16H)      Maximum Vertical Image Size __________________    =   undefined
(17H)      Display Gamma ________________________________    =   2.2 (BT.709)
(18H)      DPMS and Supported Feature(s):
			Display Type = RGB Color, Preferred Timing Mode */
	0x80,  0x80,  0x50,  0x78,  0x0A,

/*(19H-22H)  CHROMA INFO:

			Red x - 0.640 Green x - 0.300 Blue x - 0.150 White x - 0.313
			Red y - 0.330 Green y - 0.600 Blue y - 0.060 White y - 0.329
			sRGB */
	0xEE,  0x91,  0xA3,  0x54,  0x4C,  0x99,  0x26,  0x0F,  0x50,  0x54,


/* (23H)      ESTABLISHED TIMING I: */
	0x20,   /*  640 x 480 @ 60Hz (IBM,VGA) */
/*   0x00, // None Specified */

/* (24H)      ESTABLISHED TIMING II: */
	0x00, /*  None Specified */

/* (25H)      Manufacturer's Reserved Timing: */
	0x00, /*  None Specified */

/* (26H-35H)  Standard Timing Identification: */
/*            Not Used */
	0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,
	0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,

/*  Since at least one preferred timing structure is required in EDID 1.3 */

/* (36H-47H) Detailed Timing / Descriptor Block 1: */
/*  Table 69> 1280x720p (60Hz 16:9) */
	0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E,
	0x28, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E,
#if 0
/*(48H-59H) Detailed Timing / Descriptor Block 2: */
/* Table 78> 720x576p (50Hz 16:9) */
	0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0C,
	0x40, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18,
#endif
#if 1
/* (48H-59H) Detailed Timing / Descriptor Block 2: */
/*  Table 78> 720x480p (60Hz 16:9) */
	0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10,
	0x3E, 0x96, 0x00, 0x58, 0xC2, 0x21, 0x00, 0x00, 0x18,
#endif


/* (5AH-6BH) Detailed Timing / Descriptor Block 3: */
/*  Descriptor: Monitor Name string */
	0x00, 0x00, 0x00, 0xFC, 0x00, /*  header */
	VENDOR_NAME,
/* (6CH-7DH) Detailed Timing / Descriptor Block 4: */
/*          Monitor Range Limits: */
	0x00,  0x00,  0x00,  0xFD,  0x00, /*  header */
	DEFAULT_MIN_V_HZ,           /*  Min Vertical Freq */
	DEFAULT_MAX_V_HZ,           /*  Max Vertical Freq */
	DEFAULT_MIN_H_KHZ,          /*  Min Horiz. Freq */
	DEFAULT_MAX_H_KHZ,          /*  Max Horiz. Freq */
	DEFAULT_MAX_PIX_CLK_10MHZ,  /*  Pixel Clock in 10MHz */
	0x00,       /*  GTF - Not Used */
	0x0A,  0x20,  0x20,  0x20,  0x20,  0x20,  0x20, /*  padding */

/* (7EH)    Number Of Extension Blocks */
/*          Extended Flag = 1    //No Extension EDID Block(s) */
	0x01,

/* (7FH)    Check Sum */
	0x00   /*  not correct, but it is not important here */
};
#if 0
05 : 30 : 40.820 == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == =
05 : 30 : 40.820    EDID Block Number = #0
05 : 30 : 40.820    | 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
05 : 30 : 40.820 == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == =
05 : 30 : 40.821 00 :  00  ff  ff  ff  ff  ff  ff  00  4d  d9  00  f5  10  00  00  00
05 : 30 : 40.821 10 :  00  12  01  03  80  00  00  78  0a  ee  91  a3  54  4c  99  26
05 : 30 : 40.821 20 :  0f  50  54  20  00  00  01  01  01  01  01  01  01  01  01  01
05 : 30 : 40.821 30 :  01  01  01  01  01  01  01  1d  00  72  51  d0  1e  20  6e  28
05 : 30 : 40.822 40 :  55  00  c4  8e  21  00  00  1e  8c  0a  d0  90  20  40  31  20
05 : 30 : 40.822 50 :  0c  40  55  00  c4  8e  21  00  00  18  00  00  00  fc  00  4d
05 : 30 : 40.822 60 :  54  38  35  33  30  20  20  41  56  52  0a  20  00  00  00  fd
05 : 30 : 40.823 70 :  00  30  3e  17  2f  08  00  0a  20  20  20  20  20  20  01  dd
05 : 30 : 40.823 == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == =
05 : 30 : 40.823    EDID Block Number = #1
05 : 30 : 40.823    | 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
05 : 30 : 40.823 == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == =
05 : 30 : 40.824 80 :  02  03  2e  71  46  84  12  03  05  13  14  35  09  7f  07  0f
05 : 30 : 40.824 90 :  7f  07  15  07  50  3d  07  c0  57  06  00  5f  7e  01  67  7e
05 : 30 : 40.824 a0 :  00  83  4f  00  00  68  03  0c  00  20  00  b8  2d  00  8c  0a
05 : 30 : 40.824 b0 :
d0  8a  20  e0  2d  10  10  3e  96  00  c4  8e  21  00  00  18
05 : 30 : 40.825 c0 :  01  1d  80  18  71  1c  16  20  58  2c  25  00  c4  8e  21  00
05 : 30 : 40.825 d0 :  00  9e  01  1d  00  bc  52  d0  1e  20  b8  28  55  40  c4  8e
05 : 30 : 40.825 e0 :  21  00  00  1e  01  1d  80  d0  72  1c  16  20  10  2c  25  80
05 : 30 : 40.826 f0 :
c4  8e  21  00  00  9e  00  00  00  00  00  00  00  00  00  e7
05 : 30 : 40.826 == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == =
#endif
/*///////////////////////////////////////////////
//BLOCK 1 --HDMI--
///////////////////////////////////////////////// */
#if 0 /* Kenny mark it */
#ifdef EDID_SUPPORT_HD_AUDIO
UINT8 const Default_Edid_Block1[128] = {
/* 0x00 */
	0x02, 0x03,
	0x2e/*0x2B*/, 0x71,

/* Video Data block */
/* 0x04 */
	0x46,
	0x84, 0x12, 0x03,
	0x05, 0x13, 0x14,

/* Audio Data block */
/* 0x0b */
	0x35,
	0x09, 0x7f, 0x07,  /* PCM 2CH, 32~192KHz, 16/20/24Bit */
	0x0f, 0x7f, 0x07,  /* PCM 8CH, 32~192KHz, 16/20/24Bit */
	0x15, 0x07, 0x50,  /* AC3 5CH, 32~48KHz,  640Kbps */
	0x3d, 0x07, 0xc0,  /* AC3 5CH, 32~48KHz,  1536Kbps */
	/* 0x4d, 0x02, 0x00,   DSD 6ch, 44.1khz, */
	0x57, 0x06, 0x00,   /* DD+ 8ch, 44.1 ~ 48khz */
	0x5f, 0x7e, 0x01,   /* DTS-HD, 44.1~ 192 khz, */
	0x67, 0x7e, 0x00,    /* MAT 8ch, 44.1~ 192 khz, */

/* Speak Data Block */
/* 0x21 */
	0x83,
	0x5F, 0x00, 0x00,

/* Vendor Specific Data Block */
/* 0x25 */
	0x68,
	0x03, 0x0C, 0x00,   /* HDMI VSDB (0x000C03) */
	0x10, 0x00,         /* PHY Address (0x26..0x27) */
	0xB8,               /* AI, DC_36,DC_30,DC_Y444 */
	0x2D,               /* Max TMDS = 45 x 5MHz = 225MHz */
	0x00,               /* Latency Fields */

/*  Detailed Timing : Table 72> 720x480p (59.94Hz 16:9) */
/* 0x2E */
	0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10,
	0x3E, 0x96, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18,

/*  Detailed Timing : Table 70> 1920x1080i (60Hz 16:9) */
/* 0x40 */
	0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20, 0x58,
	0x2C, 0x25, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E,

/*  Detailed Timing : Table 75> 1280x720p (50Hz 16:9) */
/* 0x52 */
	0x01, 0x1D, 0x00, 0xBC, 0x52, 0xD0, 0x1E, 0x20, 0xB8,
	0x28, 0x55, 0x40, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E,

/*  Detailed Timing : Table 76> 1920x1080i (50Hz 16:9) */
/* 0x64 */
	0x01, 0x1D, 0x80, 0xD0, 0x72, 0x1C, 0x16, 0x20, 0x10,
	0x2C, 0x25, 0x80, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E,

/*   '0' Padding */
/* 0x76 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, /* 0x00, 0x00, 0x00, */

/*  Check Sum */
/* 0x7F */
	0x00        /* Not Calculate */
};

#else   /* EDID_SUPPORT_HD_AUDIO */

UINT8 const Default_Edid_Block1[128] = {
/* 0x00 */
	0x02, 0x03,
	0x25/*0x22*/, 0x71,

/* Video Data block */
/* 0x04 */
	0x46,
	0x84, 0x12, 0x03,
	0x05, 0x13, 0x14,

/* Audio Data block */
/* 0x0b */
	0x2c,
	0x0F, 0x1F, 0x07,       /* PCM 2CH, 32~96KHz, 16/20/24Bit */
	0x0F, 0x7F, 0x07,       /* PCM 8CH, 32~96KHz, 16/20/24Bit */
	0x17, 0x07, 0x50,       /* AC3 8CH, 32~ 48K, 640Kbps */
	0x3F, 0x07, 0xC0,       /* DTS 8CH, 32~ 48K, 1536Kbps */

/* Speak Data Block */
/* 0x18 */
	0x83,
	0x5F, 0x00, 0x00,

/* Vendor Specific Data Block */
/* 0x1C */
	0x68,
	0x03, 0x0C, 0x00,   /* HDMI VSDB (0x000C03) */
	0x10, 0x00,         /* PHY Address (0x26..0x27) */
/*   0x80,               //AI */
	0xB8,               /* AI, DC_36,DC_30,DC_Y444 */
	0x2D,               /* Max TMDS = 45 x 5MHz = 225MHz */
	0x00,               /*Latency Fields */

/*  Detailed Timing : Table 72> 720x480p (59.94Hz 16:9) */
/* 0x25 */
	0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10,
	0x3E, 0x96, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18,

/*  Detailed Timing : Table 70> 1920x1080i (60Hz 16:9) */
/* 0x37 */
	0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20, 0x58,
	0x2C, 0x25, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E,

/*  Detailed Timing : Table 75> 1280x720p (50Hz 16:9) */
/* 0x49 */
	0x01, 0x1D, 0x00, 0xBC, 0x52, 0xD0, 0x1E, 0x20, 0xB8,
	0x28, 0x55, 0x40, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E,

/*  Detailed Timing : Table 76> 1920x1080i (50Hz 16:9) */
/* 0x5B */
	0x01, 0x1D, 0x80, 0xD0, 0x72, 0x1C, 0x16, 0x20, 0x10,
	0x2C, 0x25, 0x80, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E,

/*   '0' Padding */
/* 0x6D */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
/*   0x00, 0x00, 0x00, */
/* Check Sum */
/* 0x7F */
	0x00        /* Not Calculate */
};

#endif  /* EDID_SUPPORT_HD_AUDIO */
#endif
/* ============================================================================== */
UINT8 const Edid_Header[8] = {
	0x00,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0x00
};

UINT8 const Edid_VendorInfo[10] = {
	EDID_MANUFACTURER_ID,
	EDID_PRODUCT_ID,
	EDID_SERIAL_NUMBER,
	EDID_WEEK,
	EDID_YEAR
};

UINT8 const Default_Edid_Block1_Header[4] = {
/* 0x00 */
	0x02, 0x03,
	0x25/*Temply*/, 0x71
};

UINT8 const Edid_VerInfo[2] = {
	0x02, 0x03
};

UINT8 const TblEstablishedTiming[3] =   {
	0x20, 0x00, 0x00        /*  640 x 480 @ 60Hz (IBM,VGA) */
};

UINT8 const RepeaterName[18] =  {
	0x00, 0x00, 0x00, 0xFC, 0x00,
	'M', 'T', 'K', ' ', ' ', 'A', 'V', 'S', 'Y', 'S', 'T', 'E', 'M'
};

/* ============================================================================== */
UINT8 Speaker_Data_Block[EDID_SPEAKER_BLOCK_LEN] = {
	0x83,
	0x0F, 0x00, 0x00

};

UINT8 Speaker_Data_Block_2CH_ONLY[EDID_SPEAKER_BLOCK_LEN] = {
	0x83,
	0x01, 0x00, 0x00

};

/*  Video Data Block resolution list
1    720P60
2    1080I60
3    1080P60
4    720P50
5    1080I50
6    1080P50
7    1080P24
8    1080P30
9    1080P25
10    720P24
11   VGA
12   480P 4:3
13   480P 16:9
14   480I 4:3
15   480I 16:9
16   576P 4:3
17   576P 16:9
18   576I 4:3
19   576I 16:9
*/

const UINT8 Video_Data_Block[EDID_VIDEO_BLOCK_LEN] = {
	0x53, /* 19//16 byes */
	0x04, 0x05, 0x90/*native 0x10,  // HD Tming place first for  3D */
	0x13, 0x14, 0x1f,
	0x20,  0x22, 0x21, 0x3C,/* for support 720P24Hz*/
	0x01,  0x02, 0x03, 0x06,  0x07,  /*  SD 60Hz timing */
	0x11,  0x12, 0x15, 0x16,            /*  SD 50Hz timing */
};


/* make sure the 2ch pcm is above on 6ch pcm, */

#ifdef EDID_SUPPORT_HD_AUDIO
UINT8  Audio_Data_Block[EDID_AUDIO_BLOCK_LEN] = {
	0x3B,
	0x09, 0x7f, 0x07,  /* PCM 2CH, 32~192KHz, 16/20/24Bit */
	0x0F, 0x1f, 0x07,  /* PCM 8CH, 32~96KHz, 16/20/24Bit */
	0x0D, 0x7f, 0x07,  /* PCM 6CH, 32~192KHz, 16/20/24Bit */
	0x15, 0x07, 0x50,  /* AC3 5CH, 32~48KHz,  640Kbps */
	0x3d, 0x07, 0xc0,  /* DTS 5CH, 32~48KHz,  1536Kbps */
	/* 0x4d, 0x02, 0x00,  //DSD 6ch, 44.1khz, */
	0x57, 0x06, 0x00,   /* DD+ 8ch, 44.1 ~ 48khz */
	0x5f, 0x7e, 0x01,   /* DTS-HD, 44.1~ 192 khz, */
	0x67, 0x7e, 0x00,    /* MAT 8ch, 44.1~ 192 khz, (DD TRUE HD) */
	0x35, 0x07, 0x50,         /* AAC 5CH, 32~48KHz,  640Kbps */
};
#if EDID_SUPPORT_PCM_ONLY_2CH == 1
UINT8 Audio_Data_Block_2CH_ONLY[EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN] = {
	0x23,
	0x09, 0x7f, 0x07,  /* PCM 2CH, 32~192KHz, 16/20/24Bit */
	/* 0x15, 0x07, 0x50,  //AC3 5CH, 32~48KHz,  640Kbps */
	/* 0x3d, 0x07, 0xc0,  //DTS 5CH, 32~48KHz,  1536Kbps */
	/* 0x4d, 0x02, 0x00,  //DSD 6ch, 44.1khz, */
	/* 0x57, 0x06, 0x00,   //DD+ 8ch, 44.1 ~ 48khz */
	/* 0x5f, 0x7e, 0x01,   //DTS-HD, 44.1~ 192 khz, */
	/* 0x67, 0x7e, 0x00,    //MAT 8ch, 44.1~ 192 khz, */
	/* 0x35, 0x07, 0x50,         //AAC 5CH, 32~48KHz,  640Kbps */
};

#else
UINT8  Audio_Data_Block_2CH_ONLY[EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN] = {
	0x35,
	0x09, 0x7f, 0x07,  /* PCM 2CH, 32~192KHz, 16/20/24Bit */
	0x15, 0x07, 0x50,  /* AC3 5CH, 32~48KHz,  640Kbps */
	0x3d, 0x07, 0xc0,  /* DTS 5CH, 32~48KHz,  1536Kbps */
	/*0x4d, 0x02, 0x00,  **** DSD 6ch, 44.1khz, */
	0x57, 0x06, 0x00,   /* DD+ 8ch, 44.1 ~ 48khz */
	0x5f, 0x7e, 0x01,   /* DTS-HD, 44.1~ 192 khz, */
	0x67, 0x7e, 0x00,    /* MAT 8ch, 44.1~ 192 khz, */
	0x35, 0x07, 0x50,         /* AAC 5CH, 32~48KHz,  640Kbps */
};
#endif
#else   /* EDID_SUPPORT_HD_AUDIO */


UINT8   Audio_Data_Block[EDID_AUDIO_BLOCK_LEN] = {
	0x2F,
	0x09, 0x7F, 0x07,       /* PCM 2CH, 32~192KHz, 16/20/24Bit */
	0x0D, 0x1F, 0x07,       /* PCM 6CH, 32~96KHz, 16/20/24Bit */
	0x15, 0x07, 0x50,       /* AC3 6CH, 32~ 48K, 640Kbps */
	0x3D, 0x07, 0xC0,       /* DTS 6CH, 32~ 48K, 1536Kbps */
	0x35, 0x07, 0x50,         /* AAC 5CH, 32~48KHz,  640Kbps */
};

#if EDID_SUPPORT_PCM_ONLY_2CH == 1
UINT8 Audio_Data_Block_2CH_ONLY[EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN] = {

	0x23,
	/* 0x09, 0x7F, 0x07,     //PCM 2CH, 32~192KHz, 16/20/24Bit */
	0x09, 0x07, 0x07,       /*PCM 2CH, 32~48KHz, 16/20/24Bit    , SONY request because wifi can't support */
	/* 0x15, 0x07, 0x50,     //AC3 6CH, 32~ 48K, 640Kbps */
	/* 0x3D, 0x07, 0xC0,     //DTS 6CH, 32~ 48K, 1536Kbps */
	/* 0x35, 0x07, 0x50,         //AAC 5CH, 32~48KHz,  640Kbps */
};

#else
UINT8 Audio_Data_Block_2CH_ONLY[EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN] = {

	0x2C,
	/* 0x09, 0x7F, 0x07,     //PCM 2CH, 32~192KHz, 16/20/24Bit */
	0x09, 0x1F, 0x07,       /* PCM 2CH, 32~96KHz, 16/20/24Bit    , SONY request because wifi can't support */
	0x15, 0x07, 0x50,       /* AC3 6CH, 32~ 48K, 640Kbps */
	0x3D, 0x07, 0xC0,       /* DTS 6CH, 32~ 48K, 1536Kbps */
	0x35, 0x07, 0x50,         /* AAC 5CH, 32~48KHz,  640Kbps */
};
#endif
#endif  /* EDID_SUPPORT_HD_AUDIO */


/*  Video Data Block resolution list
0    720P60
1    1080I60
2    1080P60
3    720P50
4    1080I50
5    1080P50
6    1080P24
7    1080P30
8    1080P25
9    720P24
10   VGA
11   480P 4:3
12   480P 16:9
13   480I 4:3
14   480I 16:9
15   576P 4:3
16   576P 16:9
17   576I 4:3
18   576I 16:9
*/


UINT8 Vendor_Data_Block_Full[EDID_VENDOR_BLOCK_FULL_LEN] = {
	/* 0x6A, //Lenghth =0x0A //0x6E, //Lenghth =0x0E */
	0x75, /* Lenghth =0x15 */
	0x03, 0x0C, 0x00,   /* HDMI VSDB (0x000C03) */
	0x10, 0x00,         /* PHY Address (0x26..0x27) */
	0xB8,               /* AI, DC_36,DC_30,DC_Y444 */
	0x2D,               /* Max TMDS = 0,not indiacate */
	0x20,               /* Latency Fields=0, HDMI_VIDEO_PRESENT=1, CNC=0 */
	/* 0x00, //video Latency */
	/* 0x00,//Audio Latency */
	/* 0x00, //interlaced_video Latency */
	/* 0x00,//interlaced_Audio Latency */
	0xC0, /* 0x80,//'3d present */
	0x0B, /* HDMI VIC LEN=0, HDMI 3D Lenghth = 0xB */
	0x01, 0x41,  /*  3D_structure_All_x */
	0x01, 0xDB,  /*  3D_MASK_All_x */
	0x26,  /* 1080P60 T&B            // 2D_VIC_order_x 3D_structure_x 3D_Detail_x */
	0x28, 0x10, /* 1080P60 SBS            // 2D_VIC_order_x 3D_structure_x 3D_Detail_x */
	0x56,  /* 1080P50 T&B            // 2D_VIC_order_x 3D_structure_x 3D_Detail_x */
	0x58, 0x10, /* 1080P50 SBS            // 2D_VIC_order_x 3D_structure_x 3D_Detail_x */
	0x90,  /* 720P24 Frame Packet // 2D_VIC_order_x 3D_structure_x 3D_Detail_x */
};

UINT8 Vendor_Data_Block_Full_NO_DEEP[EDID_VENDOR_BLOCK_FULL_LEN] = {
	/* 0x6A, //Lenghth =0x0A //0x6E, //Lenghth =0x0E */
	0x75, /* Lenghth =0x15 */
	0x03, 0x0C, 0x00,   /* HDMI VSDB (0x000C03) */
	0x10, 0x00,         /* PHY Address (0x26..0x27) */
	/* 0xB8,              **** AI, DC_36,DC_30,DC_Y444 */
	0x80,               /* AI, */
	0x00,               /* Max TMDS = 0,not indiacate */
	0x20,               /* Latency Fields=0, HDMI_VIDEO_PRESENT=1, CNC=0 */
	/* 0x00, //video Latency */
	/* 0x00,//Audio Latency */
	/* 0x00, //interlaced_video Latency */
	/* 0x00,//interlaced_Audio Latency */
	0xC0, /* 0x80,//'3d present */
	0x0B,/* HDMI VIC LEN=0, HDMI 3D Lenghth = 0xE */
	0x01, 0x41,  /*  3D_structure_All_x */
	0x01, 0xDB,  /*  3D_MASK_All_x */
	0x26,  /* 1080P60 T&B            // 2D_VIC_order_x 3D_structure_x 3D_Detail_x */
	0x28, 0x10, /* 1080P60 SBS            // 2D_VIC_order_x 3D_structure_x 3D_Detail_x */
	0x56,  /* 1080P50 T&B            // 2D_VIC_order_x 3D_structure_x 3D_Detail_x */
	0x58, 0x10, /*1080P50 SBS            // 2D_VIC_order_x 3D_structure_x 3D_Detail_x */
	0x90,  /* 720P24 Frame Packet // 2D_VIC_order_x 3D_structure_x 3D_Detail_x */

};


const UINT8 Vendor_Data_Block_4K_2K_Full[EDID_VENDOR_BLOCK_4K_2K_FULL_LEN] = {
	0x72, /* Lenghth =0x0E */
	0x03, 0x0C, 0x00,   /* HDMI VSDB (0x000C03) */
	0x10, 0x00,         /* PHY Address (0x26..0x27) */
	0xB8,               /* AI, DC_36,DC_30,DC_Y444 */
	0x3C,               /* Max TMDS = 59 x 5MHz = 300MHz */
	0xEE,               /* Latency Fields=1, HDMI_VIDEO_PRESENT=1, CNC=E */
	0x00, /* video Latency */
	0x00,/* Audio Latency */
	0x00, /* interlaced_video Latency */
	0x00,/* interlaced_Audio Latency */
	0x80,/* '3d present */
	0x80,/* HDMI VIC LEN=4, HDMI 3D Lenghth */
	0x01, /*  4k x 2k 29.97Hz */
	0x02, /*  4k x 2k 25Hz */
	0x03, /*  4k x 2k 23.98Hz */
	0x04, /*  4k x 2k 24Hz(SMPTE) */
};


const UINT8 Vendor_Data_Block[EDID_VENDOR_BLOCK_LEN] = {
	0x66,
	0x03, 0x0C, 0x00,   /* HDMI VSDB (0x000C03) */
	0x10, 0x00,         /* PHY Address (0x26..0x27) */
	0x80,               /* AI */
};

const UINT8 Vendor_Data_Block_Mini[EDID_VENDOR_BLOCK_MINI_LEN] = {
	0x66,
	0x03, 0x0C, 0x00,   /* HDMI VSDB (0x000C03) */
	0x10, 0x00,         /* PHY Address (0x26..0x27) */
	0x80,               /*  support AI = 1   jitao.shi@20100921 for Sony mini VSDB include AI = 1 */

};


const UINT8 Colorimetry_Data_Block[EDID_COLORMETRY_BLOCK_LEN] = {
	0xE3,
	0x05,/* Extended Tag code=0x05 */
	0x03, 0x01
};

const UINT8 Vcdb_Data_Block[EDID_VCDB_BLOCK_LEN] = {
	0xE2,
	0x00,/* Extended Tag code=0x00 */
	0x01/* Always OverScane */
};

const UINT8 Detail_Timing_Block[EDID_DTD_BLOCK_LEN] = {
/*  Detailed Timing : Table 72> 720x480p (59.94Hz 16:9) */
/* 0x25*/
	0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10,
	0x3E, 0x96, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18,

/*  Detailed Timing : Table 70> 1920x1080i (60Hz 16:9) */
/* 0x37*/
	0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20, 0x58,
	0x2C, 0x25, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E,

/*  Detailed Timing : Table 75> 1280x720p (50Hz 16:9) */
/* 0x49*/
	0x01, 0x1D, 0x00, 0xBC, 0x52, 0xD0, 0x1E, 0x20, 0xB8,
	0x28, 0x55, 0x40, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E,

/*  Detailed Timing : Table 76> 1920x1080i (50Hz 16:9) */
/* 0x5B*/
	0x01, 0x1D, 0x80, 0xD0, 0x72, 0x1C, 0x16, 0x20, 0x10,
	0x2C, 0x25, 0x80, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E,
};
UINT8 u1_720P60DTD[18] = {0x01, 0x1d, 0x00, 0x72, 0x51, 0xd0, 0x1e, 0x20,
	0x6e, 0x28, 0x55, 0x00, 0xc4, 0x8e, 0x21, 0x00, 0x00, 0x1e}; /* 720P60 */
UINT8 u1_1080I60DTD[18] = {0x01, 0x1d, 0x80, 0x18, 0x71, 0x1c, 0x16, 0x20,
	0x58, 0x2c, 0x25, 0x00, 0xc4, 0x8e, 0x21, 0x00, 0x00, 0x9e}; /* 1080i60 */
/* UINT8 u1_1080I60DTD[18] = {0x01,0x1d,0x80,0x18,0x71,0x1c,0x16,0x20,0x58,0x2c,
	0x25,0x00,0xc4,0x8e,0x21,0x00,0x00,0x9e};  //1080i60 */
UINT8 u1_480P60DTD[18] = {0x8c, 0x0a, 0xd0, 0x8a, 0x20, 0xe0, 0x2d, 0x10,
	0x10, 0x3e, 0x96, 0x00, 0x13, 0x8e, 0x21, 0x00, 0x00, 0x18}; /* 480P */

UINT8 u1_720P50DTD[18] = {0x01, 0x1d, 0x00, 0xbc, 0x52, 0xd0, 0x1e, 0x20, 0xb8,
	0x28, 0x55, 0x40, 0xc4, 0x8e, 0x21, 0x00, 0x00, 0x1e}; /* 720P50 */
UINT8 u1_1080I50DTD[18] = {0x01, 0x1d, 0x80, 0xd0, 0x72, 0x1c, 0x16, 0x20, 0x10,
	0x2c, 0x25, 0x80, 0xc4, 0x8e, 0x21, 0x00, 0x00, 0x9e}; /* 1080i50 */
/* UINT8 u1_1080I50DTD[18] = {0x01,0x1d,0x80,0xd0,0x72,0x1c,0x16,0x20,0x10,0x2c,
	0x25,0x80,0xc4,0x8e,0x21,0x00,0x00,0x9e};  //1080i50 */
UINT8 u1_576P50DTD[18] = {0x8c, 0x0a, 0xd0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0c,
	0x40, 0x55, 0x00, 0x13, 0x8e, 0x21, 0x00, 0x00, 0x18}; /* 576P */

UINT8 oBlock[128];
UINT8 rEDIDBuff[128];
UINT8 oEDIDBuff[128];
UINT8 modBlock[128];

UINT8 Compose_Video_Data_Block[EDID_VIDEO_BLOCK_LEN];
UINT8 Compose_Audio_Data_Block[EDID_AUDIO_BLOCK_LEN + 15]; /* add PCM 3,4,5,6,7 channel, each 3 bytes */
UINT8 Compose_Speaker_Data_Block[EDID_SPEAKER_BLOCK_LEN];
UINT8 Compose_VSDB_Data_Block[EDID_VENDOR_BLOCK_FULL_LEN];
UINT8 Compose_VSDB_Data_4K_2K_Block[EDID_VENDOR_BLOCK_4K_2K_FULL_LEN];
UINT8 Compose_Colorimetry_Data_Block[EDID_COLORMETRY_BLOCK_LEN];
UINT8 Compose_Vcdb_Data_Block[EDID_VCDB_BLOCK_LEN];

UINT8 u1HDMIIN1EDID[256];
UINT8 u1HDMIIN2EDID[256];
UINT8 u1HDMIINEDID[512];

PHY_ADDR_PARAMETER_T PHYAddr;
EDID_PARAMETER_T Edid;

HDMI_SINK_AV_CAP_T _HdmiSinkAvCap;
BOOL _fgUseAndOperation = TRUE; /* temply use Sink edid And Default EDID */
BOOL _fgVideoUseAndOperation = TRUE; /* temply use Sink edid And Default EDID */
BOOL _fgAudioUseAndOperation = FALSE;
#if EDID_SUPPORT_PCM_2CH_ONLY
BOOL _fg2chPcmOnly = TRUE;
#else
BOOL _fg2chPcmOnly = FALSE;
#endif
UINT8 _u1EDID0CHKSUM;
UINT8 _u1EDID1CHKSUM;
UINT16 _u2EDID0PA;
UINT8 _u1EDID0PAOFF;  /*  PA remap offset */
UINT16 _u2EDID1PA;
UINT8 _u1EDID1PAOFF;  /*  PA remap offset */

#if EDID_SUPPORT_NO_DEEP_COLOR_ONLY
BOOL _fgNoDeepColor = TRUE;
#else
BOOL _fgNoDeepColor = FALSE;
#endif



#define F_PIX_MAX 22275 /* 1080P 12bits */
UINT8 PHY_ADDRESS_START[5];/* max 5 VSDB PHY */

UINT16 _u2Port1PhyAddr;
UINT16 _u2Port2PhyAddr;
BOOL _fgUseModifiedPA = FALSE;
UINT16 _u2ModifiedPA = 0;
UINT8 _u1ModifiedEdidNum = 0xff;
UINT8 _u1ModifiedEdidBlock = 0xff;
BOOL  _fgEdidEnterToEdition = FALSE;

UINT32 _ui4RxEDIDFirst_16_VIC[16];
/* ============================================================================= */
UINT8 Edid_Calculate_CheckSum(UINT8 *p_block)
{
	UINT8 check_sum = 0;
	INT16 i;

	for (i = 0; i < (EDID_BLOCK_SIZE - 1); i++)
		check_sum += *(p_block + i);

	check_sum = 0x100 - check_sum;
	return check_sum;
}

/* ============================================================================= */
BOOL Edid_CheckSum_Check(UINT8 *p_block)
{
	UINT8 check_sum = 0;
	INT16 i;

	for (i = 0; i < (EDID_BLOCK_SIZE - 1); i++)
		check_sum += *(p_block + i);

	check_sum = 0x100 - check_sum;

	if (*(p_block + (EDID_BLOCK_SIZE - 1)) != check_sum)
		return FALSE;

	return TRUE;
}

/* ============================================================================= */
BOOL Edid_BL0Header_Check(UINT8 *p_block)
{
	INT16 i;

	for (i = 0; i < EDID_BL0_LEN_HEADER ; i++) {
		if (*(p_block + EDID_BL0_ADR_HEADER + i) != Default_Edid_Block0[EDID_BL0_ADR_HEADER + i])
			return FALSE;


	}


	return TRUE;
}

/* ============================================================================= */
BOOL Edid_BL0Version_Check(UINT8 *p_block)
{
	if (*(p_block + EDID_BL0_ADR_VERSION) != 1) /* only 1.x versions are allowed (not 2.0) */
		return FALSE;

	return TRUE;
}

/*============================================================================= */
/*============================================================================== */

BOOL fgDTDSupport(BYTE *prbData)
{
	UINT16 ui2HActive, ui2VActive;

	ui2HActive = (UINT16)(*(prbData + 4) & 0xf0) << 4;
	ui2HActive |= *(prbData + 2);

	ui2VActive = (UINT16)(*(prbData + 7) & 0xf0) << 4;
	ui2VActive |= *(prbData + 5);

	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
		HDMI_LOG(HDMI_LOG_DEBUG, "fgDTDSupport Check\n");
		HDMI_LOG(HDMI_LOG_DEBUG, "ui2HActive = 0x%x, ui2VActive = 0x%x\n", ui2HActive, ui2VActive);

	}

	if ((ui2HActive == 0x780) && ((ui2VActive == 0x21c) || (ui2VActive == 0x438))) /* 1080I60  1080P60 */
		return TRUE;

	if ((ui2HActive == 0x2D0) && ((ui2VActive == 0x1E0) || (ui2VActive == 0x240))) /* 480P60, 576P */
		return TRUE;

	if ((ui2HActive == 0x500) && (ui2VActive == 0x2D0)) /* 720P60, 720P50 */
		return TRUE;

	if ((ui2HActive == 0x280) && (ui2VActive == 0x1E0)) /*  640x480  //jitao.shi@20100921 for support VGA timing */
		return TRUE;

	return FALSE;
}


BOOL fgSVDSupport(BYTE *prbData)
{
	if (((*prbData) == 0x2) || ((*prbData) == 0x3) || ((*prbData) == 0x20)
		|| ((*prbData) == 0x11) || ((*prbData) == 0x12) || ((*prbData) == 0x13)
		|| ((*prbData) == 0x14) || ((*prbData) == 0x1E) || ((*prbData) == 0x04)
		|| ((*prbData) == 0x05) || ((*prbData) == 0x10))
		return TRUE;
	else
		return FALSE;
}
void ComposeEdidBlock0(UINT8 *pr_rBlock, UINT8 *pr_oBlock)
{

	UINT8 i;
	UINT8 rEdidAddr, oEdidAddr;
/* #ifdef DUMP_EDID */
	/* UINT8 rCount; */
/* #endif */
#if CONFIG_DRV_CUSTOM_JXF
	UINT8 u1DTDNum = 0;
#else
	UINT16 u2PixClock;
#endif
	/* Union2Byte    temp2B; */
	UINT32  sink_vid;

	sink_vid = (_HdmiSinkAvCap.ui4_sink_cea_ntsc_resolution | _HdmiSinkAvCap.ui4_sink_cea_pal_resolution);

	for (i = 0x00; i <= 0x13; i++)
		*(pr_oBlock + i) = Default_Edid_Block0[i];

	for (i = 0x14; i <= 0x18; i++)         /* basic Display Prams */
		*(pr_oBlock + i) = *(pr_rBlock + i);

	for (i = 0x19; i <= 0x22; i++)         /* Chromaticity */
		*(pr_oBlock + i) = *(pr_rBlock + i);

	for (i = 0x23; i <= 0x35; i++)
		*(pr_oBlock + i) = Default_Edid_Block0[i];



	rEdidAddr = EDID_BL0_ADR_DTDs;  /* 0x36 */
	oEdidAddr = rEdidAddr;

	while (1) {

#if  CONFIG_DRV_CUSTOM_JXF

		if (oEdidAddr < (EDID_BL0_ADR_DTDs + 18 + 18)) { /* first two DTD blcok */

			/* if (fgDTDSupport((pr_oBlock+oEdidAddr ))==TRUE) */
			if (sink_vid & SINK_720P60) {
				for (i = 0; i <= 17; i++)      /* Detailed Timing */
					*(pr_oBlock + oEdidAddr + i) = u1_720P60DTD[i];
			} else if (sink_vid & SINK_1080I60) {
				for (i = 0; i <= 17; i++)    /* Detailed Timing */
					*(pr_oBlock + oEdidAddr + i) = u1_1080I60DTD[i];
			} else if (sink_vid & SINK_720P60) {
				for (i = 0; i <= 17; i++)    /* Detailed Timing */
					*(pr_oBlock + oEdidAddr + i) = u1_720P50DTD[i];
			} else if (sink_vid & SINK_1080I50) {
				for (i = 0; i <= 17; i++)    /* Detailed Timing */
					*(pr_oBlock + oEdidAddr + i) = u1_1080I50DTD[i];
			} else if (sink_vid & SINK_480P) {
				for (i = 0; i <= 17; i++)    /* Detailed Timing */
					*(pr_oBlock + oEdidAddr + i) = u1_480P60DTD[i];
			} else if (sink_vid & SINK_576P) {
				for (i = 0; i <= 17; i++)    /* Detailed Timing */
					*(pr_oBlock + oEdidAddr + i) = u1_576P50DTD[i];
			} else {
				if (u1DTDNum == 0) {
					for (i = 0; i <= 17; i++)    /* Detailed Timing */
						*(pr_oBlock + oEdidAddr + i) = u1_720P60DTD[i];

					u1DTDNum = 1;
				} else {
					for (i = 0; i <= 17; i++)    /* Detailed Timing */
						*(pr_oBlock + oEdidAddr + i) = u1_480P60DTD[i];
				}
			}

			oEdidAddr += 18;            /* Increase address for one DTD */
		} else { /*  last two dtd use the default */

			/* if (fgDTDSupport((pr_oBlock+oEdidAddr ))==TRUE) */
			for (i = 0; i <= 17; i++)    /* Detailed Timing */
				*(pr_oBlock + oEdidAddr + i) = Default_Edid_Block0[oEdidAddr + i];

			oEdidAddr += 18;            /* Increase address for one DTD */
		}

#else

		u2PixClock = (*(pr_rBlock + rEdidAddr + 1) << 8) | (*(pr_rBlock + rEdidAddr));


		if ((u2PixClock <= F_PIX_MAX) && u2PixClock) {  /* 16500 */

			/* if (fgDTDSupport((pr_oBlock+oEdidAddr ))==TRUE) */
			if (fgDTDSupport((pr_rBlock + rEdidAddr)) == TRUE) {
				for (i = 0; i <= 17; i++)      /* Detailed Timing */
					*(pr_oBlock + oEdidAddr + i) = *(pr_rBlock + rEdidAddr + i);
			} else {
				for (i = 0; i <= 17; i++)    /* Detailed Timing */
					*(pr_oBlock + oEdidAddr + i) = 0;
			}

			oEdidAddr += 18;            /* Increase address for one DTD */
		} else if (u2PixClock == 0x0000) {
			if (*(pr_rBlock + rEdidAddr + 3) == EDID_MONITOR_RANGE_DTD) { /* 00 00 00 FD */
				for (i = 0; i <= 17; i++)      /* Monitor Range Limit
					*(pr_oBlock + oEdidAddr + i) = *(pr_rBlock + rEdidAddr + i);*/

				oEdidAddr += 18;            /* Increase address for one DTD */
			} else if (*(pr_rBlock + rEdidAddr + 3) == EDID_MONITOR_NAME_DTD) { /* 00 00 00 FC */
				for (i = 0; i <= 17; i++)      /*Monitor Name */
					*(pr_oBlock + oEdidAddr + i) = Default_Edid_Block0[0x5a + i];

				oEdidAddr += 18;            /* Increase address for one DTD */
			} else {
				for (i = 0; i <= 17; i++)
					*(pr_oBlock + oEdidAddr + i) = *(pr_rBlock + rEdidAddr + i);

				oEdidAddr += 18;            /* Increase address for one DTD */
			}
		}

		if (oEdidAddr >= (END_1stPAGE_DESCR_ADDR - 18))     /* 0x7e */
			break;

#endif
		rEdidAddr += 18;

		if (rEdidAddr >= END_1stPAGE_DESCR_ADDR)    /* 0x7e */
			break;


	}


	if (_fgAudioUseAndOperation == TRUE)
		*(pr_oBlock + EDID_BL0_ADR_EXTENSION_NMB) = *(pr_rBlock + EDID_BL0_ADR_EXTENSION_NMB);
	else
		*(pr_oBlock + EDID_BL0_ADR_EXTENSION_NMB) = 0x01;

	*(pr_oBlock + EDID_ADR_CHECK_SUM) = Edid_Calculate_CheckSum(pr_oBlock);

	/*
	    if(fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
	    {
	    UTIL_Printf("\r\n") ;
	    for( i = 0 ; i < 0x80 ; i+=0x10 )
	    {
			for( rCount = 0 ; rCount < 0x10 ; rCount++ )
			{
				UTIL_Printf("%02x ",*(pr_oBlock+rCount+i));
			}
			UTIL_Printf("\r\n");
	    }
	    }
	*/

}


/*============================================================================== */
void Default_Edid_BL0_Write(void)
{
	UINT8 Addr;

/*====================================== */
/*  Block 0 */

	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
		HDMI_LOG(HDMI_LOG_DEBUG, "Default_Edid_BL0_Write");

	for (Addr = 0; Addr < 127; Addr++)
		oBlock[Addr] = Default_Edid_Block0[Addr];


	oBlock[EDID_ADR_CHECK_SUM] = Edid_Calculate_CheckSum(&oBlock[0]);


	vWriteEDIDBlk0(EEPROM0, oBlock);
	vWriteEDIDBlk0(EEPROM1, oBlock);

}

/* ====================================== */
void Default_Edid_BL1_Write(void)
{
	UINT8 Addr, oriAddr, i, j;
	UINT8 u1EdidNum, PhyAddInx, u1PAHigh, u1PALow;
	UINT8 u1PhyAddInx, u1DownSTRM;
	UINT16 ui2_pa, u2Port1PhyAddr, u2Port2PhyAddr;

/*====================================== */
/*  Block 1 */
/*  Hdmi.bLatency = true; */
/*  Hdmi.ALatency = 0; */
	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
		HDMI_LOG(HDMI_LOG_DEBUG, "Default_Edid_BL1_Write");

	for (Addr = 0; Addr < 4; Addr++)
		oBlock[Addr] = Default_Edid_Block1_Header[Addr];


	for (i = 0; i < EDID_VIDEO_BLOCK_LEN ; i++, Addr++)
		oBlock[Addr] = Video_Data_Block[i];

	/*
	if(fgOnlySupportPcm2ch() == TRUE)
	{
	  for(i=0; i<EDID_AUDIO_BLOCK_LEN ; i++)
	  Audio_Data_Block[i]=0;

	  for(i=0; i< sizeof(Audio_Data_Block_2CH_ONLY) ; i++)
	  Audio_Data_Block[i]= Audio_Data_Block_2CH_ONLY[i];

	}
	*/


/* for(i=0; i<EDID_AUDIO_BLOCK_LEN ; i++, Addr ++) */
/* for(i=0; i< (Audio_Data_Block[0]&0x1f)+1 ; i++, Addr ++) */
/* oBlock[Addr] = Audio_Data_Block[i]; */
	if (fgOnlySupportPcm2ch() == TRUE) {
		for (i = 0; (i < EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN) && (Addr < 128) ; i++, Addr++)
			oBlock[Addr] = Audio_Data_Block_2CH_ONLY[i];
	} else {
		for (i = 0; (i < EDID_AUDIO_BLOCK_LEN) && (Addr < 128); i++, Addr++)
			oBlock[Addr] = Audio_Data_Block[i];
	}

	if (fgOnlySupportPcm2ch() == TRUE) {
		for (i = 0; (i < EDID_SPEAKER_BLOCK_LEN) && (Addr < 128) ; i++, Addr++)
			oBlock[Addr] = Speaker_Data_Block_2CH_ONLY[i];
	} else {
		for (i = 0; (i < EDID_SPEAKER_BLOCK_LEN) && (Addr < 128) ; i++, Addr++)
			oBlock[Addr] = Speaker_Data_Block[i];
	}

	oriAddr = Addr;

	u1PhyAddInx = Addr + 4;

	if (fgOnlySupportNoDeep() == TRUE) {
		for (i = 0; i < EDID_VENDOR_BLOCK_FULL_LEN ; i++)
			Vendor_Data_Block_Full[i] = Vendor_Data_Block_Full_NO_DEEP[i];
	}

	for (u1EdidNum = EEPROM0; u1EdidNum <= EEPROM1; u1EdidNum++) {
		switch (u1EdidNum) {
		case  EEPROM0:

#if CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX && \
	(EDID_SUPPORT_MODE == TWO_EXT_MODE || EDID_SUPPORT_MODE == MIX_MODE) && EDID_SUPPORT_QHD == 1
for (i = 0; i < EDID_VENDOR_BLOCK_4K_2K_FULL_LEN; i++, Addr++) {
	if (Addr < 128) {
		if ((i == 4) || (i == 5)) { /* PA start address */
			if (_fgUseModifiedPA == TRUE) {
				if (i == 4)
					oBlock[Addr] = (_u2ModifiedPA >> 8) & 0xff;
					if (i == 5)
						oBlock[Addr] = _u2ModifiedPA & 0xff;
			} else
				oBlock[Addr] = Vendor_Data_Block_4K_2K_Full[i];
		} else
			oBlock[Addr] = Vendor_Data_Block_4K_2K_Full[i];
	}
}

#else

for (i = 0; i < EDID_VENDOR_BLOCK_FULL_LEN ; i++, Addr++) {
	if (Addr < 128) {
		if ((i == 4) || (i == 5)) { /* PA start address */
			if (_fgUseModifiedPA == TRUE) {
				if (i == 4)
					oBlock[Addr] = (_u2ModifiedPA >> 8) & 0xff;
				if (i == 5)
					oBlock[Addr] = _u2ModifiedPA & 0xff;
			} else
				oBlock[Addr] = Vendor_Data_Block_Full[i];
		} else
			oBlock[Addr] = Vendor_Data_Block_Full[i];
	}
}

#endif

			oBlock[2] = Addr;/* Update data offset */


			for (i = 0; (Addr + 18) < 128 ; i += 18) {
				if ((i + 18) >= EDID_DTD_BLOCK_LEN)
					break;

				for (j = 0; (j < 18) && (Addr < 128); j++, Addr++)
					oBlock[Addr] = Detail_Timing_Block[i + j];
			}

			for (; Addr < 128; Addr++)
				oBlock[Addr] =  0;

#if CONFIG_DRV_CUSTOM_JSN
			ifcon_rw_registry(REGISTRY_CFG_DOWNSTREAM_EDID, (UINT8 *)&u1DownSTRM, 1, TRUE, FALSE);
			ifcon_rw_registry(REGISTRY_HDMI_PA, (UINT8 *)&ui2_pa, sizeof(UINT16), TRUE, FALSE);
#else
			u1DownSTRM = 0;
			ui2_pa = 0;
#endif

			for (PhyAddInx = 0; PhyAddInx < 16; PhyAddInx += 4) {
				if (((ui2_pa >> PhyAddInx) & 0x0f) != 0)
					break;
			}

			if (PhyAddInx != 0) {
				if (PhyAddInx >= 4)
					PhyAddInx -= 4;


				u2Port1PhyAddr = (ui2_pa | (1 << PhyAddInx));
				u2Port2PhyAddr = (ui2_pa | (2 << PhyAddInx));
			} else {
				if ((u1DownSTRM == 0) && (ui2_pa == 0xFFFF)) {
					u2Port1PhyAddr = 0x1100;
					u2Port2PhyAddr = 0x1200;
/* Printf("[HDMI_RX_EDID_DBG] debug 1"); */
				} else {
					u2Port1PhyAddr = 0xFFFF;
					u2Port2PhyAddr = 0xFFFF;
				}
			}

			u1PAHigh = (UINT8)(u2Port1PhyAddr >> 8);
			u1PALow = (UINT8)(u2Port1PhyAddr & 0xFF);

			if ((u1PhyAddInx + 1) < sizeof(oBlock)) { /* for klocwork issue */
				oBlock[u1PhyAddInx] = u1PAHigh;
				oBlock[u1PhyAddInx + 1] = u1PALow;
			}

			_u2Port1PhyAddr = u2Port1PhyAddr;

			if (1) { /* fgIsTxDetectHotPlugIn() == TRUE) //081103 */
				if ((u1PhyAddInx + 2) < sizeof(oBlock)) { /* for klocwork issue */
					oBlock[u1PhyAddInx + 2] &= 0x80;  /* Del eDeep Color Info */
				}
			}


			oBlock[EDID_ADR_CHECK_SUM] = Edid_Calculate_CheckSum(&oBlock[0]);
			_u1EDID0CHKSUM = oBlock[EDID_ADR_CHECK_SUM];
			_u2EDID0PA = u2Port1PhyAddr;
			_u1EDID0PAOFF = u1PhyAddInx;
			vWriteEDIDBlk1(EEPROM0, oBlock);
			break;

		case  EEPROM1:
			Addr = oriAddr;

for (i = 0; (i < EDID_VENDOR_BLOCK_FULL_LEN) && (Addr < 128) ; i++, Addr++) {
	if (Addr < 128) {
		if ((i == 4) || (i == 5)) { /* PA start address */
			if (_fgUseModifiedPA == TRUE) {
				if (i == 4)
					oBlock[Addr] = (_u2ModifiedPA >> 8) & 0xff;
				if (i == 5)
					oBlock[Addr] = _u2ModifiedPA & 0xff;
			} else
				oBlock[Addr] = Vendor_Data_Block_Full[i];
		} else
			oBlock[Addr] = Vendor_Data_Block_Full[i];
	}
}

			oBlock[2] = Addr;/* Update data offset */

			for (i = 0; (Addr + 18) < 128 ; i += 18) {
				if ((i + 18) >= EDID_DTD_BLOCK_LEN)
					break;

				for (j = 0; (j < 18) && (Addr < 128); j++, Addr++)
					oBlock[Addr] = Detail_Timing_Block[i + j];
			}

			for (; Addr < 128; Addr++)
				oBlock[Addr] =  0;

#if CONFIG_DRV_CUSTOM_JSN
			ifcon_rw_registry(REGISTRY_CFG_DOWNSTREAM_EDID, (UINT8 *)&u1DownSTRM, 1, TRUE, FALSE);
			ifcon_rw_registry(REGISTRY_HDMI_PA, (UINT8 *)&ui2_pa, sizeof(UINT16), TRUE, FALSE);
#else
			u1DownSTRM = 0;
			ui2_pa = 0;
#endif

			for (PhyAddInx = 0; PhyAddInx < 16; PhyAddInx += 4) {
				if (((ui2_pa >> PhyAddInx) & 0x0f) != 0)
					break;
			}

			if (PhyAddInx != 0) {
				if (PhyAddInx >= 4)
					PhyAddInx -= 4;


				u2Port1PhyAddr = (ui2_pa | (1 << PhyAddInx));
				u2Port2PhyAddr = (ui2_pa | (2 << PhyAddInx));
			} else {
				if ((u1DownSTRM == 0) && (ui2_pa == 0xFFFF)) {
					u2Port1PhyAddr = 0x1100;
					u2Port2PhyAddr = 0x1200;
/*  Printf("[HDMI_RX_EDID_DBG] debug 1"); */
				} else {
					u2Port1PhyAddr = 0xFFFF;
					u2Port2PhyAddr = 0xFFFF;
				}
			}

			u1PAHigh = (UINT8)(u2Port2PhyAddr >> 8);
			u1PALow = (UINT8)(u2Port2PhyAddr & 0xFF);

			if ((u1PhyAddInx + 1) < sizeof(oBlock)) { /* for klocwork issue */
				oBlock[u1PhyAddInx] = u1PAHigh;
				oBlock[u1PhyAddInx + 1] = u1PALow;
			}

			_u2Port1PhyAddr = u2Port2PhyAddr;

			if (1) { /* fgIsTxDetectHotPlugIn() == TRUE) //081103 */
				if ((u1PhyAddInx + 2) < sizeof(oBlock)) { /* for klocwork issue */
					oBlock[u1PhyAddInx + 2] &= 0x80;  /* Del eDeep Color Info */
				}
			}

			oBlock[EDID_ADR_CHECK_SUM] = Edid_Calculate_CheckSum(&oBlock[0]);

			_u1EDID1CHKSUM = oBlock[EDID_ADR_CHECK_SUM];
			_u2EDID1PA = u2Port2PhyAddr;
			_u1EDID1PAOFF = u1PhyAddInx;

			vWriteEDIDBlk1(EEPROM1, oBlock);
			break;

		default:
			break;
		} /* switch(u1EdidNum) */



		if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
			HDMI_LOG(HDMI_LOG_DEBUG, "After  default EdidBlock1 u1EdidNum%d Page1 EDID ===\r\n", u1EdidNum);

			/* TIL_Printf("    0  1  2  3  4  5  6  7 - 8  9  A  B  C  D  E  F\r\n"); */
			for (j = 0; j < 127; j += 8)

				HDMI_LOG(HDMI_LOG_DEBUG, "Add= %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
				j, oBlock[j], oBlock[j + 1], oBlock[j + 2], oBlock[j + 3],
				oBlock[j + 4], oBlock[j + 5], oBlock[j + 6], oBlock[j + 7]);

		}


	} /* for(u1EdidNum=EEPROM0; u1EdidNum <= EEPROM1; u1EdidNum++) */







	Edid.bBlock1Err = TRUE;
	Edid.bDownDvi = TRUE;

	PHYAddr.Origin = 0x0000;
	PHYAddr.Dvd = 0x1000;
	PHYAddr.Sat = 0x2000;
	PHYAddr.Tv = 0x3000;
	PHYAddr.Else = 0x4000;

	Edid.PHYLevel = 1;      /* 081104 */
#if 0
	TxDev.bDnBasicAudEn = 0;
	TxDev.bDnSupportVidMode = (oBlock[3] & 0x30) >> 4;
	TxDev.bDnHDMIMode = false ; /* 20081015 for astro DVI mode */
#endif


}

/******************************************************************************/
BOOL ParseHDMIEDID(UINT8 *prBlock)
{
#if 0
	UINT8 End;
	UINT8 rCount;/* , oCount; */
	UINT8 tag, tagCount;
/* #ifdef DUMP_EDID */
	UINT8 i;
/* #endif */

	BOOL vendor_flag;

	vendor_flag = FALSE;

	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
		HDMI_LOG(HDMI_LOG_DEBUG, "\r\n");

		for (i = 0 ; i < 0x80 ; i += 0x10) {
			for (rCount = 0 ; rCount < 0x10 ; rCount++)
				HDMI_LOG(HDMI_LOG_DEBUG, "%02x ", *(prBlock + rCount + i));
		}
	}

	if (*(prBlock + 0) != 2)
		return FALSE;

	if (*(prBlock + 1) < 3)
		return FALSE ; /*  for futhur EDID supporting of SimplyHD test. */

	End = *(prBlock + 2); /*  CEA description. */

	for (rCount = 4; rCount < End;) {
		tag = (*(prBlock + rCount) >> 5);
		tagCount = (*(prBlock + rCount) & 0x1f);
		rCount++;

		if (tag == 0x03) {
			if ((*(prBlock + rCount) == 0x03) &&
				(*(prBlock + rCount + 1) == 0x0C) && (*(prBlock + rCount + 2) == 0x00))
				vendor_flag = TRUE;

			break;
		}

		rCount += tagCount; /*  ignore the remaind. */
	}

	return vendor_flag;
#endif
	return TRUE;
}

void ComposeVideoBlock(UINT8 u1VideoAndOperation, UINT8 u1AudioAndOperation)
{
	return;
#if 0
	UINT32 sink_vid, sink_native_vid, ui4Temp = 0;
	UINT8 TotalLen, i, j;
	UINT8 temp[EDID_VIDEO_BLOCK_LEN];


	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
		HDMI_LOG(HDMI_LOG_DEBUG, "ComposeVideoBlock\n");

	sink_vid = (_HdmiSinkAvCap.ui4_sink_dtd_ntsc_resolution |
		_HdmiSinkAvCap.ui4_sink_org_cea_ntsc_resolution |
		_HdmiSinkAvCap.ui4_sink_dtd_pal_resolution |
		_HdmiSinkAvCap.ui4_sink_org_cea_pal_resolution);
	sink_native_vid = (_HdmiSinkAvCap.ui4_sink_native_ntsc_resolution |
		_HdmiSinkAvCap.ui4_sink_native_pal_resolution);

	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
		HDMI_LOG(HDMI_LOG_DEBUG, "sink_vid=0x%x, u1VideoAndOperation =%d\n",
			sink_vid, u1VideoAndOperation);
		HDMI_LOG(HDMI_LOG_DEBUG, "_HdmiSinkAvCap.b_sink_support_hdmi_mode=%d\n",
			_HdmiSinkAvCap.b_sink_support_hdmi_mode);
		HDMI_LOG(HDMI_LOG_DEBUG, "HdmiSinkAvCap.ui4_sink_native_ntsc_resolution = 0x%x\n",
			_HdmiSinkAvCap.ui4_sink_native_ntsc_resolution);
		HDMI_LOG(HDMI_LOG_DEBUG, "HdmiSinkAvCap.ui4_sink_native_pal_resolution = 0x%x\n",
			_HdmiSinkAvCap.ui4_sink_native_pal_resolution);
	}

	/* TotalLen=    Video_Data_Block[0]&0x1f; **** video Block, VID, not include Header */
	TotalLen =    EDID_VIDEO_BLOCK_LEN - 1;

	for (i = 0; i < TotalLen + 1; i++)
		Compose_Video_Data_Block[i] = 0; /* Include header */

	for (i = 0; i < 0x10; i++)
		_ui4RxEDIDFirst_16_VIC[i] = 0;

	i = 1; /* Skip Header */

	while (TotalLen) {
		switch (Video_Data_Block[i] & 0x7f) {
		case 1:
			if (u1VideoAndOperation) {
				if (sink_vid & SINK_VGA) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_VGA)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */
				} else {
					if ((_HdmiSinkAvCap.b_sink_support_hdmi_mode == FALSE) &&
						(u1AudioAndOperation == FALSE))
						/* DVI and "Speaker" or "speaker+HDMI" */
						Compose_Video_Data_Block[i] =  1; /* 640x480P VGA */
					else
						Compose_Video_Data_Block[i] = 0;
				}

			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];



			break;

		case 2:/* 720x480P 4:3 */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_480P_4_3) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_480P_4_3)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 3:
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_480P) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_480P)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 4:/* 720P60 */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_720P60) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_720P60)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */
				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 5:/* 1080i60 */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_1080I60) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_1080I60)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */
				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 6:/* 480i */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_480I_4_3) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_480I_4_3)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 7:/* 480i */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_480I) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_480I)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 16:/* 1080P60 */
			if (u1VideoAndOperation) { /* t should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_1080P60) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_1080P60)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */
				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 17:/* 576P */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_576P_4_3) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_576P_4_3)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 18:
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_576P) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_576P)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 19:/* 720P50 */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_720P50) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_720P50)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */
				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 20:/* 1080I50 */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_1080I50) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_1080I50)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;


		case 21:/* 576i */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_576I_4_3) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_576I_4_3)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 22:
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_576I) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_576I)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;


		case 31:/* 1080P50 */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if ((sink_vid & SINK_1080P50)) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_1080P50)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 32:/* 1080P24 */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if ((sink_vid & SINK_1080P24) || (sink_vid & SINK_1080P23976)) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if ((sink_native_vid & SINK_1080P24) || (sink_native_vid & SINK_1080P23976))
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 60:/* 720P24 */
			if (u1VideoAndOperation) { /*It should be decided By UI, Original EDID and default EDID */
				if ((sink_vid & SINK_720P24) || (sink_vid & SINK_720P23976)) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if ((sink_native_vid & SINK_720P24) || (sink_native_vid & SINK_720P23976))
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;

		case 34:/* 1080P30 */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if ((sink_vid & SINK_1080P30) || (sink_vid & SINK_1080P2997)) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if ((sink_native_vid & SINK_1080P30) || (sink_native_vid & SINK_1080P2997))
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

		case 33:/* 1080P25 */
			if (u1VideoAndOperation) { /* It should be decided By UI, Original EDID and default EDID */
				if (sink_vid & SINK_1080P25) {
					Compose_Video_Data_Block[i] =  Video_Data_Block[i] & 0x7f;

					if (sink_native_vid & SINK_1080P25)
						Compose_Video_Data_Block[i] |= 0x80;/* add native */

				} else
					Compose_Video_Data_Block[i] = 0;
			} else
				Compose_Video_Data_Block[i] =  Video_Data_Block[i];

			break;


		default:
			Compose_Video_Data_Block[i] = 0;
			break;
		}

		i++;
		TotalLen--;

	}


	for (i = 0; i < EDID_VIDEO_BLOCK_LEN; i++)
		temp[i] = 0;

	j = 1;
	TotalLen = 0;

	for (i = 1; i < EDID_VIDEO_BLOCK_LEN; i++) { /* Not include header */
		if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
			/* UTIL_Printf("i=%d, j=%d, C=%d, T=%d\n", i, j,  Compose_Video_Data_Block[i], temp[j]); */
		}

		if (Compose_Video_Data_Block[i] != 0) {
			temp[j] = Compose_Video_Data_Block[i];
			TotalLen++;
			j++;

/*  get rx video block VIC order,  used for compose vsdb 3D info */
			if (i <= 0x10) {

				switch (Compose_Video_Data_Block[i] & 0x7f) {
				case  1:
					ui4Temp = 0;
					ui4Temp |= SINK_VGA;
					break;

				case  6:
				case  7:
					ui4Temp = SINK_480I;
					break;

				case  2:
				case  3:
					ui4Temp = SINK_480P;
					break;

				case  14:
				case  15:
					ui4Temp = SINK_480P_1440;
					break;

				case  4:
					ui4Temp = SINK_720P60;
					break;

				case  5:
					ui4Temp = SINK_1080I60;
					break;

				case  21:
				case  22:
					ui4Temp = SINK_576I;
					break;

				case  16:
					ui4Temp = SINK_1080P60;
					break;

				case  17:
				case  18:
					ui4Temp = SINK_576P;
					break;

				case  29:
				case  30:
					ui4Temp = SINK_576P_1440;
					break;

				case  19:
					ui4Temp = SINK_720P50;
					break;

				case  20:
					ui4Temp = SINK_1080I50;
					break;

				case  31:
					ui4Temp = SINK_1080P50;
					break;

				case  32:
					ui4Temp = 0;
					ui4Temp |= SINK_1080P24;
					ui4Temp |= SINK_1080P23976;
					break;

				case  33:
					ui4Temp = SINK_1080P25;
					break;

				case  34:
					ui4Temp = 0;
					ui4Temp |= SINK_1080P30;
					ui4Temp |= SINK_1080P2997;

					break;

				case  60:
					ui4Temp = 0;
					ui4Temp |= SINK_720P24;
					ui4Temp |= SINK_720P23976;

					break;

				default:
					break;

				}

				_ui4RxEDIDFirst_16_VIC[i - 1] = ui4Temp;

				if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
					HDMI_LOG(HDMI_LOG_DEBUG,
					"[Rx EDID] _ui4RxEDIDFirst_16_VIC[%d] = 0x%x", i - 1, ui4Temp);

			}



		}

	}

/*	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
UTIL_Printf("TotalLen=%d\n", TotalLen);
	} */

	temp[0] = ((0x02 << 5) | (TotalLen)); /* Update Header */

	for (i = 0; i < (TotalLen + 1); i++) {
		Compose_Video_Data_Block[i] = temp[i];

		if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
			/* UTIL_Printf("i=%d C=%d, T=%d\n", i,  Compose_Video_Data_Block[i], temp[i]); */
		}
	}

#endif
}

void vCEAAudioDataAndOperation(UINT8 *prData, UINT8 bCEALen, UINT8 *poBlock, UINT8 *poCount)
{
	return;
#if 0
	BYTE bTemp, bIdx;
	BYTE bLengthSum;
	BYTE bType, bNo, i;
	UINT8 TotalLen = 0 , dec = 0, chan = 0, sink_dec = 0, sink_chan = 0, dec_support = 0, head_add = 0;
	UINT8 data_byte2 = 0, data_byte3 = 0, sink_byte2 = 0, sink_byte3 = 0, head_start = 0, sad_len = 0;
	UINT8 pcm_max_chan = 1, PCM2CHADD = 0; /* 2 channel */
	UINT8 u1Audio_Data_Block[EDID_AUDIO_BLOCK_LEN];

	HDMI_LOG(HDMI_LOG_INFO, "vCEAAudioDataAndOperation\n");
	dec = 0;
	chan = 0;
	pcm_max_chan = 0;
	sink_dec = 0;
	sink_chan = 0;

	for (i = 0; i < sizeof(u1Audio_Data_Block) ; i++)
		u1Audio_Data_Block[i] = 0;


	if (fgOnlySupportPcm2ch() == TRUE) { /* 2ch only, update audio block */

		for (i = 0; i < sizeof(Audio_Data_Block_2CH_ONLY) ; i++)
			u1Audio_Data_Block[i] = Audio_Data_Block_2CH_ONLY[i];

	} else {

		for (i = 0; i < sizeof(Audio_Data_Block) ; i++)
			u1Audio_Data_Block[i] = Audio_Data_Block[i];
	}


	/* TotalLen=    Audio_Data_Block[0]&0x1f; */
	if (fgOnlySupportPcm2ch() == TRUE)
		TotalLen =    EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN - 1;
	else
		TotalLen =    EDID_AUDIO_BLOCK_LEN - 1;

	i = 1;

	while (TotalLen) { /* check default PCM how many channels are supportted */
		if (i < sizeof(u1Audio_Data_Block)) { /* for klocwork issue */
			dec = (u1Audio_Data_Block[i] >> 3) & 0x0f; /* Default Support */

			if (dec == 0x01) {
				chan = (u1Audio_Data_Block[i] & 0x07);

				if (chan > pcm_max_chan)
					pcm_max_chan = chan;
			}
		}

		i += 3;

		if (TotalLen >= 3) /* for reduce warning */
			TotalLen -= 3;
		else
			TotalLen = 0;


	}


	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]pcm_max_chan =%d\n", pcm_max_chan);

	while (bCEALen > 0) {
		/*  Step 1: get 1st data block type & total number of this data type */
		bTemp = *prData;
		bType = bTemp >> 5; /*  bit[7:5] */
		bNo = bTemp & 0x1F; /*  bit[4:0] */

		if (bType == 0x01) { /*  Audio data block */


			head_add = 0;
			sad_len = 0;

			for (bIdx = 0; bIdx < (bNo / 3); bIdx++) {
				bLengthSum = bIdx * 3;

				dec_support = 0;

				/* TotalLen=    Audio_Data_Block[0]&0x1f; */
				if (fgOnlySupportPcm2ch() == TRUE)
					TotalLen =    EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN - 1;
				else
					TotalLen =    EDID_AUDIO_BLOCK_LEN - 1;

				i = 1;

			while (TotalLen) { /* check default support */
				if (i < sizeof(u1Audio_Data_Block)) { /* for klocwork issue */
						dec = (u1Audio_Data_Block[i] >> 3) & 0x0f; /* Default Support */
						/* if(dec == 0x01) */
						/* chan = pcm_max_chan;//7; //max 8ch for default */
						/* else */
						chan = (u1Audio_Data_Block[i] & 0x07);

						sink_dec = (*(prData + bLengthSum + 1) >> 3) & 0x0f;
						sink_chan = (*(prData + bLengthSum + 1) & 0x07);

						HDMI_LOG(HDMI_LOG_DEBUG,
						"[hdmi RX]sink_chan =%d, chan =%d\n", sink_chan, chan);

				if (dec ==  sink_dec) {
					if (dec != 1) {
							dec_support = 1;

					if ((i + 2) < sizeof(u1Audio_Data_Block)) {
							/* for klocwork issue */
							data_byte2 = u1Audio_Data_Block[i + 1];
							data_byte3 = u1Audio_Data_Block[i + 2];
							}
							break;
						}
					if ((chan >= sink_chan) ||
							((chan >= 5) && (chan <= sink_chan))) {
							dec_support = 1;

					if ((i + 2) < sizeof(u1Audio_Data_Block)) {
							/* for klocwork issue */
							data_byte2 = u1Audio_Data_Block[i + 1];
							data_byte3 = u1Audio_Data_Block[i + 2];
							}
							break;
					}
				}
			}

					i += 3;

					if (TotalLen >= 3) /* for reduce warning */
						TotalLen -= 3;
					else
						TotalLen = 0;

				}


				if (dec_support) {
					if (head_add == 0) {
						head_start = *poCount;
						*(poBlock + *poCount) = *prData;
						/*  Printf("[HDMI RX]SAD Head =0x%x\n", *(poBlock+ *poCount)); */
						*poCount = (*poCount + 1);
						head_add = 1;
					}

					sink_byte2 = *(prData + bLengthSum + 2);
					sink_byte3 = *(prData + bLengthSum + 3);

					if (chan >= sink_chan)
						*(poBlock + *poCount) = (dec << 3) | sink_chan;
					else
						*(poBlock + *poCount) = (dec << 3) | chan;

					/*  Printf("[HDMI RX]SAD BYTE1 =0x%x\n", *(poBlock+ *poCount)); */
					*poCount = (*poCount + 1);

					*(poBlock + *poCount) = (sink_byte2 & data_byte2);
					/*  Printf("[HDMI RX]SAD BYTE2 =0x%x\n", *(poBlock+ *poCount)); */
					*poCount = (*poCount + 1);

					if (sink_dec == 0x01) /* PCM */
						*(poBlock + *poCount) = (sink_byte3 & data_byte3);
					else if ((sink_dec >= 0x02) && (sink_dec >= 0x08)) /* Ac3~ATRC */
						*(poBlock + *poCount) = sink_byte3;
					else
						*(poBlock + *poCount) = sink_byte3;

					/* Printf("[HDMI RX]SAD BYTE3 =0x%x\n", *(poBlock+ *poCount)); */
					*poCount = (*poCount + 1);

					if ((_HdmiSinkAvCap.ui1_sink_org_pcm_ch_sampling[0] == 0) &&
					(sink_dec == 0x01) && (PCM2CHADD == 0)) {
						HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX] ADD SAD PCM 2CH");
						*(poBlock + *poCount) = (dec << 3) | 1; /*  PCM 2CH */
						*poCount = (*poCount + 1);

						*(poBlock + *poCount) = (sink_byte2 & 0x1F);
						/*  0x1F : PCM 2CH support sample rates. */

						*poCount = (*poCount + 1);

						*(poBlock + *poCount) = (sink_byte3 & data_byte3);
						*poCount = (*poCount + 1);

						PCM2CHADD = 1;
						sad_len += 3;
					}

					sad_len += 3;
				} /* if(dec_support) */


			} /* for(bIdx = 0; bIdx < bNo/3; bIdx++) */

			if (head_add) {
				*(poBlock + head_start) = (0x01 << 5) | sad_len; /* update header again */
			}

		}


		/*  re-assign the next data block address */
		prData += (bNo + 1); /*  '1' means the tag byte */

		if (bCEALen >= (bNo + 1))
			bCEALen -= (bNo + 1);
		else
			bCEALen = 0;

	} /* while(bCEALen) */

#endif
}

void ComposeAudioBlock(UINT8 u1AndOperation)
{
	return;

#if 0
	UINT8 TotalLen, i, j, k, idx, dec, chan, sink_dec;
	UINT8 temp[EDID_AUDIO_BLOCK_LEN + 15]; /* add PCM 3,4,5,6,7 channel */
	UINT8 u1Audio_Data_Block[EDID_AUDIO_BLOCK_LEN];

	dec = 0;
	chan = 0;

	for (i = 0; i < (EDID_AUDIO_BLOCK_LEN + 15); i++) {
		Compose_Audio_Data_Block[i] = 0;
		temp[i] = 0;
	}

	for (i = 0; i < EDID_AUDIO_BLOCK_LEN ; i++)
		u1Audio_Data_Block[i] = 0;

	if (fgOnlySupportPcm2ch() == TRUE) { /* 2ch only, update audio block */

		for (i = 0; i < sizeof(Audio_Data_Block_2CH_ONLY) ; i++)
			u1Audio_Data_Block[i] = Audio_Data_Block_2CH_ONLY[i];

	} else {

		for (i = 0; i < sizeof(Audio_Data_Block) ; i++)
			u1Audio_Data_Block[i] = Audio_Data_Block[i];

	}

	sink_dec = _HdmiSinkAvCap.ui2_sink_aud_dec;

	/* TotalLen=    Audio_Data_Block[0]&0x1f; */
	if (fgOnlySupportPcm2ch() == TRUE)
		TotalLen =    EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN - 1;
	else
		TotalLen =    EDID_AUDIO_BLOCK_LEN - 1;

	temp[0] =  u1Audio_Data_Block[0];

	i = 1;
	k = 1;

	while (TotalLen) {
		if (i < sizeof(u1Audio_Data_Block)) { /* for klocwork issue */
			dec = (u1Audio_Data_Block[i] >> 3) & 0x0f; /* Default Support */
			chan = (u1Audio_Data_Block[i] & 0x07) + 1;
		}

		switch (dec) {
		case 0x01:/* PCM */

	/* Default And read EDID */
	if (u1AndOperation) { /* Here, we need add UI check */
		if (chan == 8) { /* skip 2ch */
			/* Deal with PCM */
			for (idx = 0; idx < 7; idx++) { /* 2ch to 8ch */
				if ((_HdmiSinkAvCap.ui1_sink_org_pcm_ch_sampling[idx] != 0)) {
					if ((k + 2) < sizeof(temp)) { /* for klocwork issue */
					temp[k] = ((0x1 << 3) | (idx + 1));
					temp[k + 1] =
					_HdmiSinkAvCap.ui1_sink_org_pcm_ch_sampling[idx];
					temp[k + 2] =
					_HdmiSinkAvCap.ui1_sink_org_pcm_bit_size[idx];
				}
				k += 3;
			}
		}
	}





			} /* if(u1AndOperation) */
			else {
				if (((k + 2) < sizeof(temp)) &&
					((i + 2) < sizeof(u1Audio_Data_Block))) { /* for klocwork issue */
					temp[k]  = u1Audio_Data_Block[i];
					temp[k + 1] =  u1Audio_Data_Block[i + 1];
					temp[k + 2] =  u1Audio_Data_Block[i + 2];
				}

				k += 3;
			}



			break;

		case 0x02:/* Ac3 */
		case 0x06:/* AAC */
		case 0x07:/* DTS */
		case 0x09:/* DSD */
		case 0x0A:/* DD+ */
		case 0x0B:/* DTS-HD */
		case 0x0C:/* MAT */

			/* Default And read EDID */
			if (u1AndOperation) { /* Here, we need add UI check */
				if (((sink_dec & HDMI_SINK_AUDIO_DEC_AC3) && (dec == 0x02)) ||
				    ((sink_dec & HDMI_SINK_AUDIO_DEC_AAC) && (dec == 0x06)) ||
				    ((sink_dec & HDMI_SINK_AUDIO_DEC_DTS) && (dec == 0x07)) ||
				    ((sink_dec & HDMI_SINK_AUDIO_DEC_DSD) && (dec == 0x09)) ||
				    ((sink_dec & HDMI_SINK_AUDIO_DEC_DOLBY_PLUS) && (dec == 0x0A)) ||
				    ((sink_dec & HDMI_SINK_AUDIO_DEC_MAT_MLP) && (dec == 0x0C))
				   ) {
					if (((k + 2) < sizeof(temp)) &&
						((i + 2) < sizeof(u1Audio_Data_Block))) { /*for klocwork issue */
						temp[k]  = u1Audio_Data_Block[i];
						temp[k + 1] =  u1Audio_Data_Block[i + 1];
						temp[k + 2] =  u1Audio_Data_Block[i + 2];
					}

					k += 3;
				} else {
					if ((k + 2) < sizeof(temp)) { /*for klocwork issue */
						temp[k] = 0;
						temp[k + 1] =  0;
						temp[k + 2] =  0;
					}

					k += 3;

				}

			} else {
				if (((k + 2) < sizeof(temp)) &&
					((i + 2) < sizeof(u1Audio_Data_Block))) { /* for klocwork issue */
					temp[k]  = u1Audio_Data_Block[i];
					temp[k + 1] =  u1Audio_Data_Block[i + 1];
					temp[k + 2] =  u1Audio_Data_Block[i + 2];
				}

				k += 3;
			}

			break;




		default:
			break;

		}

		i += 3;

		if (TotalLen >= 3) /* for reduce warning */
			TotalLen -= 3;
		else
			TotalLen = 0;
	}

	TotalLen = 0;
	j = 1;

	for (i = 1; i < (EDID_AUDIO_BLOCK_LEN + 15); i += 3) {
		if (temp[i] != 0) {
			Compose_Audio_Data_Block[j] = temp[i];
			Compose_Audio_Data_Block[j + 1] = temp[i + 1];
			Compose_Audio_Data_Block[j + 2] = temp[i + 2];
			TotalLen += 3;
			j += 3;
		}

	}

	Compose_Audio_Data_Block[0] = (0x01 << 5) | TotalLen;
#endif
}



void vCEASpkAndOperation(UINT8 *prData, UINT8 bCEALen, UINT8 *poBlock, UINT8 *poCount)
{

	BYTE bTemp;
	BYTE bType, bNo, sink_speak;



	HDMI_LOG(HDMI_LOG_INFO, "vCEASpkAndOperation\n");

	while (bCEALen > 0) {
		/*  Step 1: get 1st data block type & total number of this data type */
		bTemp = *prData;
		bType = bTemp >> 5; /*  bit[7:5] */
		bNo = bTemp & 0x1F; /*  bit[4:0] */

		if ((bType == 0x04) && (bNo == 3)) { /* speaker allocation tag code, 0x04 */

			*(poBlock + *poCount) = *prData;/* header */

			*poCount = *poCount + 1;

			sink_speak = *(prData + 1);

			/* Printf("[HDMI RX]Sink SPK = 0x%x, default SPK =0x%x\n",
			sink_speak, Speaker_Data_Block[1]); */
			if (fgOnlySupportPcm2ch() == TRUE)
				*(poBlock + *poCount) =  Speaker_Data_Block_2CH_ONLY[1] & sink_speak;
			else
				*(poBlock + *poCount) =  Speaker_Data_Block[1] & sink_speak;

			*poCount = *poCount + 1;
			*(poBlock + *poCount) =  *(prData + 2);
			*poCount = *poCount + 1;

			*(poBlock + *poCount) =  *(prData + 3);
			*poCount = *poCount + 1;

		}

		/*  re-assign the next data block address */
		prData += (bNo + 1); /*  '1' means the tag byte */

		if (bCEALen >= (bNo + 1))
			bCEALen -= (bNo + 1);
		else
			bCEALen = 0;

	} /* while(bCEALen) */
}
void ComposeSpeakerAllocate(UINT8 u1AndOperation)
{
	UINT8 sink_speak, i;
	UINT8 u1speakalltemp[EDID_SPEAKER_BLOCK_LEN];

	sink_speak = _HdmiSinkAvCap.ui1_sink_spk_allocation;

	if (fgOnlySupportPcm2ch() == TRUE) {
		for (i = 0; i < EDID_SPEAKER_BLOCK_LEN ; i++)
			u1speakalltemp[i] = Speaker_Data_Block_2CH_ONLY[i];
	} else {
		for (i = 0; i < EDID_SPEAKER_BLOCK_LEN ; i++)
			u1speakalltemp[i] = Speaker_Data_Block[i];
	}

	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]sink_speak = 0x%x", sink_speak);

	if (u1AndOperation) {
		if (sink_speak != 0)
			Compose_Speaker_Data_Block[0] = u1speakalltemp[0];
		else
			Compose_Speaker_Data_Block[0] = 0;


		if (sink_speak != 0)
			Compose_Speaker_Data_Block[1] = u1speakalltemp[1] & sink_speak;
		else
			Compose_Speaker_Data_Block[1] = 0; /* Speaker_Data_Block[1]; */

		if (sink_speak != 0) {
			Compose_Speaker_Data_Block[2] = u1speakalltemp[2];
			Compose_Speaker_Data_Block[3] = u1speakalltemp[3];
		} else {
			Compose_Speaker_Data_Block[2] = 0;
			Compose_Speaker_Data_Block[3] = 0;
		}
	} else {
		Compose_Speaker_Data_Block[0] = u1speakalltemp[0];
		Compose_Speaker_Data_Block[1] = u1speakalltemp[1];
		Compose_Speaker_Data_Block[2] = u1speakalltemp[2];
		Compose_Speaker_Data_Block[3] = u1speakalltemp[3];
	}


}

BYTE aTxEDIDVSDBHeader[3] = {0x03, 0x0c, 0x00};

UINT8 u1BitCnt(UINT32 u4Data)
{
	UINT8 u1Conter = 0, u1Tmp;

	for (u1Tmp = 0; u1Tmp < 32; u1Tmp++) {
		if (u4Data & (1 << u1Tmp))
			u1Conter++;
	}

	return u1Conter;

}

BOOL fgSupport4K2K(void)
{
#if 0
	BOOL fgReturn = FALSE;
	UINT32 u4CEASinkCap;

	u4CEASinkCap =  _HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic;
#if 1   /* for debug */

	if (u4CEASinkCap & (SINK_2160P_23_976HZ | SINK_2160P_24HZ |
		SINK_2160P_25HZ | SINK_2160P_29_97HZ | SINK_2160P_30HZ | SINK_2161P_24HZ))
		fgReturn = TRUE;

#else
#if  (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT3363) && \
((EDID_SUPPORT_MODE == TWO_EXT_MODE) || (EDID_SUPPORT_MODE == MIX_MODE)) && (EDID_SUPPORT_QHD == 1)

	if ((Edid.Number == EEPROM0) && (u4CEASinkCap & (SINK_2160P_23_976HZ |
		SINK_2160P_24HZ | SINK_2160P_25HZ | SINK_2160P_29_97HZ
		| SINK_2160P_30HZ | SINK_2161P_24HZ)))
		fgReturn = TRUE;

#endif
#endif


	return fgReturn;
#endif
	return FALSE;
}



void ComposeVSDB(UINT8 u1VideoAndOperation, UINT8 u1AudioAndOperation,
	UINT8 *prData, UINT8 bCEALen, UINT8 *poBlock, UINT8 *poCount)
{
}

void ComposeMiniVSDB(UINT8 u1AndOperation)
{
	UINT8 i;
	/* UINT8 temp[EDID_VENDOR_BLOCK_FULL_LEN]; */



	for (i = 0; i < EDID_VENDOR_BLOCK_MINI_LEN; i++)
		Compose_VSDB_Data_Block[i] = Vendor_Data_Block_Mini[i];

	for (i = 0; i < 5; i++)
		PHY_ADDRESS_START[i] = 0;

	/* Compose_VSDB_Data_Block[4]=  (_HdmiSinkAvCap.ui2_sink_cec_address >> 8)&0xff; */
	/* Compose_VSDB_Data_Block[5]=  _HdmiSinkAvCap.ui2_sink_cec_address &0xff; */

/*  Printf(" _HdmiSinkAvCap.b_sink_hdmi_video_present =%d\n", _HdmiSinkAvCap.b_sink_hdmi_video_present); */
/*  Printf(" _HdmiSinkAvCap.b_sink_3D_present =%d\n", _HdmiSinkAvCap.b_sink_3D_present); */
}


void ComposeColoriMetry(UINT8 u1AndOperation)
{
	UINT8 sink_xvYcc = 0, sink_metadata = 0;

	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_XV_YCC601)
		sink_xvYcc |= 0x01;

	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_XV_YCC709)
		sink_xvYcc |= 0x02;

	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_METADATA0)
		sink_metadata |= 0x01;

	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_METADATA1)
		sink_metadata |= 0x02;

	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_METADATA2)
		sink_metadata |= 0x04;


	if (u1AndOperation) {
		Compose_Colorimetry_Data_Block[0] = Colorimetry_Data_Block[0];
		Compose_Colorimetry_Data_Block[1] = Colorimetry_Data_Block[1];
		Compose_Colorimetry_Data_Block[2] = Colorimetry_Data_Block[2] & sink_xvYcc;
		Compose_Colorimetry_Data_Block[3] = Colorimetry_Data_Block[3] & sink_metadata;
	} else {
		Compose_Colorimetry_Data_Block[0] = Colorimetry_Data_Block[0];
		Compose_Colorimetry_Data_Block[1] = Colorimetry_Data_Block[1];
		Compose_Colorimetry_Data_Block[2] = Colorimetry_Data_Block[2];
		Compose_Colorimetry_Data_Block[3] = Colorimetry_Data_Block[3];

	}

}



void ComposeVCDB(UINT8 u1AndOperation)
{
	if (u1AndOperation) {
		Compose_Vcdb_Data_Block[0] = Vcdb_Data_Block[0];
		Compose_Vcdb_Data_Block[1] = Vcdb_Data_Block[1];
		Compose_Vcdb_Data_Block[2] = Vcdb_Data_Block[2];
	} else {
		Compose_Vcdb_Data_Block[0] = Vcdb_Data_Block[0];
		Compose_Vcdb_Data_Block[1] = Vcdb_Data_Block[1];
		Compose_Vcdb_Data_Block[2] = Vcdb_Data_Block[2];

	}

}



BOOL ComposeEdidBlock1(UINT8 *prBlock, UINT8 *poBlock)
{
	UINT8 rEnd, i, DataBlkLen = 0;
	UINT8 rCount, oCount;/* , oTempCout; */
	UINT16 u2PixClock;
	UINT8 *prCEAStar, CEALen = 0;
	/* UINT8 tag, tagCount, TotalLen,dec, chan; */


	BOOL audio_flag, vendor_flag;
	BOOL speaker_flag;/* , bSupportsAI; */

	audio_flag = FALSE;
	vendor_flag = FALSE;
	speaker_flag = FALSE;
	/* bSupportsAI = FALSE; */

	rEnd = *(prBlock + 2); /*  Original EDID CEA description. */
	prCEAStar = (prBlock + 4);

	if (rEnd >= 4)
		CEALen = rEnd - 4;

	/* TxDev.bDnBasicAudEn = (*(prBlock+3) & 0x40) >> 6; */
	/* TxDev.bDnSupportVidMode = (*(prBlock+3)&0x30)>>4; */

	DataBlkLen = 0;
	oCount = 0;

	for (i = 0; i < 4; i ++, oCount++)
		*(poBlock + oCount) = *(prBlock + i);

	if (_fgAudioUseAndOperation == FALSE) /* Audio output is HDMI+Speaket or Speaker */
		*(poBlock + 1) = 3;

	if (_HdmiSinkAvCap.b_sink_support_hdmi_mode == FALSE) { /* DVI */
		*(poBlock + 3) = (*(poBlock + 3)) & (0x8f); /* clear YcbCr444 and Ycbcr422 */

		if (_fgAudioUseAndOperation == FALSE) /* Audio output is HDMI+Speaket or Speaker */
			*(poBlock + 3) = (*(poBlock + 3)) | (1 << 6); /* Set basic audio */

	}



	/* The following is for Data Blcok process */
	/* Process video data Block */
	HDMI_LOG(HDMI_LOG_DEBUG, "_fgVideoUseAndOperation =%d\n", _fgVideoUseAndOperation);
	HDMI_LOG(HDMI_LOG_DEBUG, "_fgAudioUseAndOperation =%d\n", _fgAudioUseAndOperation);

	if (_fgVideoUseAndOperation)
		ComposeVideoBlock(1, _fgAudioUseAndOperation);
	else
		ComposeVideoBlock(0, _fgAudioUseAndOperation);

	if ((Compose_Video_Data_Block[0] & 0x1f) != 0) { /* SVD exist */
		DataBlkLen = DataBlkLen + (Compose_Video_Data_Block[0] & 0x1f) + 1; /* Add Video block length */

		for (i = 0; i < ((Compose_Video_Data_Block[0] & 0x1f) + 1); i++,  oCount++)
			*(poBlock + oCount) = Compose_Video_Data_Block[i];

	}



	/* Process Audio data Block */
	if (_fgAudioUseAndOperation) {
		vCEAAudioDataAndOperation(prCEAStar, CEALen, poBlock, &oCount);
		/* ComposeAudioBlock(1); */

	} else {
		ComposeAudioBlock(0);


		if ((Compose_Audio_Data_Block[0] & 0x1f) != 0) { /* SAD exist */
			DataBlkLen = DataBlkLen + (Compose_Audio_Data_Block[0] & 0x1f) + 1; /* Add Audio block length */

			for (i = 0; i < ((Compose_Audio_Data_Block[0] & 0x1f) + 1); i++,  oCount++)
				*(poBlock + oCount) = Compose_Audio_Data_Block[i];

			audio_flag = TRUE;
		}
	}


	/* Process Speaker Allocation */
	if (_fgAudioUseAndOperation) {
		/* ComposeSpeakerAllocate(1); */
		vCEASpkAndOperation(prCEAStar, CEALen, poBlock, &oCount);
	} else {
		ComposeSpeakerAllocate(0);

		if ((Compose_Speaker_Data_Block[0] & 0x1f) != 0) {
			DataBlkLen = DataBlkLen + (Compose_Speaker_Data_Block[0] & 0x1f) + 1;
			/* Add Speaker Aloocate block length */

			for (i = 0; i < ((Compose_Speaker_Data_Block[0] & 0x1f) + 1); i++,  oCount++)
				*(poBlock + oCount) = Compose_Speaker_Data_Block[i];

			speaker_flag = TRUE;
		}
	}

	/* Process VSDB */
	if (_HdmiSinkAvCap.b_sink_support_hdmi_mode == TRUE) { /*HDMI mode, not DVI */
		if (_fgVideoUseAndOperation)
			ComposeVSDB(1, _fgAudioUseAndOperation, prCEAStar, CEALen, poBlock, &oCount);
		else
			ComposeVSDB(0, _fgAudioUseAndOperation, prCEAStar, CEALen, poBlock, &oCount);

		/*
			if(fgSupport4K2K())
			{
					PHYAddr.Origin = (Compose_VSDB_Data_4K_2K_Block[4] << 8) |
					Compose_VSDB_Data_4K_2K_Block[5];
					Edid.PHYPoint = oCount + 4;

					DataBlkLen = DataBlkLen+(Compose_VSDB_Data_4K_2K_Block[0]&0x1f)+1;
					for(i=0; i< ((Compose_VSDB_Data_4K_2K_Block[0]&0x1f)+1); i++,    oCount++)
					*(poBlock+oCount) = Compose_VSDB_Data_4K_2K_Block[i];

		    }
			else
			{

		       PHYAddr.Origin = (Compose_VSDB_Data_Block[4]<<8)|Compose_VSDB_Data_Block[5];
		       Edid.PHYPoint = oCount + 4;

		       DataBlkLen = DataBlkLen+(Compose_VSDB_Data_Block[0]&0x1f)+1;
		       for(i=0; i< ((Compose_VSDB_Data_Block[0]&0x1f)+1); i++,  oCount++)
		       *(poBlock+oCount) = Compose_VSDB_Data_Block[i];
				}
		*/

	} else {

		if (_fgAudioUseAndOperation) { /* HDMI Bypass */
			/* ComposeMiniVSDB(1); */
			PHYAddr.Origin = 0;
			Edid.PHYPoint = 0;

			for (i = 0; i < 5; i++)
				PHY_ADDRESS_START[i] = 0;
		} else {
			ComposeMiniVSDB(0);
			PHYAddr.Origin = (Compose_VSDB_Data_Block[4] << 8) | Compose_VSDB_Data_Block[5];
			Edid.PHYPoint = oCount + 4;
			PHY_ADDRESS_START[0] = Edid.PHYPoint;
			DataBlkLen = DataBlkLen + (Compose_VSDB_Data_Block[0] & 0x1f) + 1; /* Add VSDB length */

			for (i = 0; i < ((Compose_VSDB_Data_Block[0] & 0x1f) + 1); i++,  oCount++)
				*(poBlock + oCount) = Compose_VSDB_Data_Block[i];
		}

	}

	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]PHYAddr.Origin = 0x%x\n", PHYAddr.Origin);
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Edid.PHYPoint = %d\n", Edid.PHYPoint);
	}

	vendor_flag = TRUE;
	/* Process   Colorimetry */
#if 1  /* temply mark, Sony4G does not support this */

	if ((_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_XV_YCC601) ||
	    (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_XV_YCC709)
	   ) {
		if (_fgVideoUseAndOperation)
			ComposeColoriMetry(1);
		else
			ComposeColoriMetry(0);

		DataBlkLen = DataBlkLen + (Compose_Colorimetry_Data_Block[0] & 0x1f) + 1;
		/*Add Speaker Aloocate block length */

		for (i = 0; i < ((Compose_Colorimetry_Data_Block[0] & 0x1f) + 1); i++,  oCount++)
			*(poBlock + oCount) = Compose_Colorimetry_Data_Block[i];

	}

#endif
	/* Process VCDB(video Capability) */

	/* Finally , update Byte number offset again */

	*(poBlock + 2) = oCount; /* CEA description. */

	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Compose DTD rEnd =0x%x, oCount =0x%x", rEnd, oCount);


	for (rCount = rEnd; (oCount + 18) < 128; rCount += 18) {
		if ((rCount + 18) >= 128) {
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]rCount+18)>=128");
			break;
		}

		u2PixClock = (*(prBlock + rCount + 1) << 8) | (*(prBlock + rCount));

		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]u2PixClock =%d", u2PixClock);

		if ((u2PixClock <= F_PIX_MAX) && u2PixClock) {


			if (fgDTDSupport(prBlock + rCount) == TRUE) { /* kenny add 2010/6/28 */
				HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]Supported DTD");

				for (i = 0; i < 18; i++, oCount++)
					*(poBlock + oCount) = *(prBlock + rCount + i);

			} else
				HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]UnSupported DTD");

		}



	}

	for (; oCount < 128; oCount++)
		*(poBlock + oCount) = 0;


	return TRUE;
}

/***********************************************************************************/
void EdidProcessing(void)
{


	UINT8 PhyAddInx;
	BOOL b4BlockEdid;
	UINT8   Extention;
	UINT16 u2Port1PhyAddr = 0, u2Port2PhyAddr = 0, l;
#ifdef _PRINT_EDID_READ_WRITE_
	UINT8 j;
#endif

/*  u8 rEDIDBuff[128]; */
/*  u8 oEDIDBuff[128]; */

	if (_fgEdidEnterToEdition == TRUE)
		/* kenny 2010/9/1 , if user use CLI "modedid" to edit EDID, then it will not do process by HPD */
		return;

	HDMI_LOG(HDMI_LOG_DEBUG, "EdidProcessing()");

	Edid.bBlock0Err = FALSE;
	Edid.bBlock1Err = FALSE;
	Edid.bDownDvi = FALSE;


/*  TxDev.bDnHDMIMode = true ; //20081015 for Astro VA-1809 DVI mode

=====================================
  Block0
=====================================*/
	if (1) { /* vGetTxPortEdid(rEDIDBuff,0,128) == TRUE)   Ke Xu */

		if (Edid_CheckSum_Check(rEDIDBuff) == FALSE) {
			if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
				HDMI_LOG(HDMI_LOG_ERROR, "Edid CheckSum Err\r\n");


			Default_Edid_BL0_BL1_Write();
			return;

		}

		if (Edid_BL0Header_Check(rEDIDBuff) == FALSE) {
			if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
				HDMI_LOG(HDMI_LOG_ERROR, "Edid Header Err\r\n");

			Default_Edid_BL0_BL1_Write();
			return;

		}

		if (Edid_BL0Version_Check(rEDIDBuff) == FALSE) {
			if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
				HDMI_LOG(HDMI_LOG_ERROR, "Edid Version Err\r\n");

			Default_Edid_BL0_BL1_Write();
			return;

		}


		Extention = rEDIDBuff[EDID_BL0_ADR_EXTENSION_NMB];
		b4BlockEdid = FALSE;
		HDMI_LOG(HDMI_LOG_DEBUG, "Extention Block = %d\n", Extention);

		switch (Extention) {
		case    0:
		default:

			ComposeEdidBlock0(rEDIDBuff, oEDIDBuff);
			vWriteEDIDBlk0(EEPROM0, oEDIDBuff);
			vWriteEDIDBlk0(EEPROM1, oEDIDBuff);
			vEDIDCreateBlock1();
			return;

		case    1:
			break;

		case    2:
		case    3:
			if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
				HDMI_LOG(HDMI_LOG_DEBUG, "Edid 4Block Detect");

			b4BlockEdid = TRUE;
			break;
		}

	} else {
		if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
			HDMI_LOG(HDMI_LOG_DEBUG, "Edid0 Read Error ===");

		Default_Edid_BL0_BL1_Write();
		return;
	}

	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
		HDMI_LOG(HDMI_LOG_DEBUG, "TX Read Block 0 EDID ===");

		for (j = 0; j < 127; j += 8)

			HDMI_LOG(HDMI_LOG_DEBUG, "Add= %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
			j, rEDIDBuff[j], rEDIDBuff[j + 1], rEDIDBuff[j + 2], rEDIDBuff[j + 3],
			rEDIDBuff[j + 4], rEDIDBuff[j + 5], rEDIDBuff[j + 6], rEDIDBuff[j + 7]);

	}


	ComposeEdidBlock0(rEDIDBuff, oEDIDBuff);


	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
		HDMI_LOG(HDMI_LOG_DEBUG, "After  ComposeEdidBlock0 Page0 EDID ===\r\n");

		/* UTIL_Printf("    0  1  2  3  4  5  6  7 - 8  9  A  B  C  D  E  F\r\n"); */
		for (j = 0; j < 127; j += 8)

			HDMI_LOG(HDMI_LOG_DEBUG, "Add= %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
			j, oEDIDBuff[j], oEDIDBuff[j + 1], oEDIDBuff[j + 2], oEDIDBuff[j + 3],
			oEDIDBuff[j + 4], oEDIDBuff[j + 5], oEDIDBuff[j + 6], oEDIDBuff[j + 7]);

	}

	vWriteEDIDBlk0(EEPROM0, oEDIDBuff);
	vWriteEDIDBlk0(EEPROM1, oEDIDBuff);


/****************************************************
  Block1
bug, max7088 disable -->  if(b4BlockEdid == true) */
	{
		/* UINT8 tmpBuff[128]; */
		int i;
		BOOL GotHDMIFlag;

		GotHDMIFlag = FALSE;

		for (i = 1 ; i < 4 && i <= Extention ; i++) {
			if (GotHDMIFlag == FALSE) {
				if (1) { /* vGetTxPortEdid(rEDIDBuff,i,128) == TRUE) block 2 */
					if (ParseHDMIEDID(rEDIDBuff) == TRUE) {
						GotHDMIFlag = TRUE;

					if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
						HDMI_LOG(HDMI_LOG_DEBUG, "TX Read Block %2d EDID ===", i);
					for (j = 0; j < 127; j += 8)
						HDMI_LOG(HDMI_LOG_DEBUG,
						"Add= %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
						j, rEDIDBuff[j], rEDIDBuff[j + 1], rEDIDBuff[j + 2],
						rEDIDBuff[j + 3], rEDIDBuff[j + 4], rEDIDBuff[j + 5],
						rEDIDBuff[j + 6], rEDIDBuff[j + 7]);
						}
					} else
						GotHDMIFlag = FALSE;

				} else {
					if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
						HDMI_LOG(HDMI_LOG_ERROR, "Edid1 Read Error ===\r\n");

					Default_Edid_BL1_Write();
					return;
				}
			} else {
				/*   ReadEDID(tmpBuff,i/2,i%2,128); */
			}
		}

		if (GotHDMIFlag == FALSE) {
			Default_Edid_BL1_Write();
			return;
		}
	}




	for (Edid.Number = 0; Edid.Number < HDMI_INPUT_COUNT; Edid.Number++) {
		switch (Edid.Number) {
		case    EEPROM0:
			/* oEDIDBuff[Edid.PHYPoint] = (PHYAddr.Dvd >> 8); */
			/* oEDIDBuff[Edid.PHYPoint + 1] = (PHYAddr.Dvd & 0x00ff); */

			ComposeEdidBlock1(rEDIDBuff, oEDIDBuff);


			if (Edid.PHYPoint != 0) { /* Mean that is not DVI */

				for (PhyAddInx = 0; PhyAddInx < 16; PhyAddInx += 4) {
					if (((PHYAddr.Origin >> PhyAddInx) & 0x0f) != 0)
						break;
				}

				if (PhyAddInx != 0) {
					if (PhyAddInx >= 4)
						PhyAddInx -=   4;


					u2Port1PhyAddr = (PHYAddr.Origin | (1 << PhyAddInx));
					u2Port2PhyAddr = (PHYAddr.Origin | (2 << PhyAddInx));
				} else {
					u2Port1PhyAddr = 0xffff;
					u2Port2PhyAddr = 0xffff;
				}

				if (_fgUseModifiedPA == TRUE) { /* for CLI debug mode */

					u2Port1PhyAddr = _u2ModifiedPA;

					u2Port2PhyAddr = _u2ModifiedPA;
				}

				_u2Port1PhyAddr =  u2Port1PhyAddr;
				_u2Port2PhyAddr  = u2Port2PhyAddr;

				if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
					HDMI_LOG(HDMI_LOG_DEBUG, "u2Port1PhyAddr =0x%x\n", u2Port1PhyAddr);
					HDMI_LOG(HDMI_LOG_DEBUG, "u2Port2PhyAddr =0x%x\n", u2Port2PhyAddr);
				}
			} /* if(Edid.PHYPoint != 0) */


			for (l = 0; l < 5; l++) {
				if (PHY_ADDRESS_START[l] != 0) {
					HDMI_LOG(HDMI_LOG_DEBUG, "1.PHY_ADDRESS_START[%d]=0x%x\n",
						l, PHY_ADDRESS_START[l]);
					j = PHY_ADDRESS_START[l];

					if (j < sizeof(oEDIDBuff)) /*  for klocwork issue */
						oEDIDBuff[j] = (u2Port1PhyAddr >> 8) & 0xff;

					j = j + 1;

					if (j < sizeof(oEDIDBuff)) /*  for klocwork issue */
						oEDIDBuff[j] = u2Port1PhyAddr & 0xff;
				}

			}

			oEDIDBuff[EDID_ADR_CHECK_SUM] = Edid_Calculate_CheckSum(oEDIDBuff);

#ifdef _PRINT_EDID_
			/* Printf("PHYAddr.Dvd = 0x%x\r\n",PHYAddr.Dvd); */
			HDMI_LOG(HDMI_LOG_ERROR, "u2Port1PhyAddr = 0x%x\r\n", u2Port1PhyAddr);
#endif
/*  to do write to EEPROM */

			_u1EDID0CHKSUM = oEDIDBuff[EDID_ADR_CHECK_SUM];
			_u2EDID0PA = u2Port1PhyAddr;
			_u1EDID0PAOFF = Edid.PHYPoint;

			vWriteEDIDBlk1(EEPROM0, oEDIDBuff);

			if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
				HDMI_LOG(HDMI_LOG_DEBUG, "u2Port1PhyAddr = 0x%x", u2Port1PhyAddr);

			break;

		case    EEPROM1:
			/* oEDIDBuff[Edid.PHYPoint] = (PHYAddr.Sat >> 8); */
			/* oEDIDBuff[Edid.PHYPoint + 1] = (PHYAddr.Sat & 0x00ff); */

			ComposeEdidBlock1(rEDIDBuff, oEDIDBuff);


			if (Edid.PHYPoint != 0) { /* Mean that is not DVI */

				for (PhyAddInx = 0; PhyAddInx < 16; PhyAddInx += 4) {
					if (((PHYAddr.Origin >> PhyAddInx) & 0x0f) != 0)
						break;
				}

				if (PhyAddInx != 0) {
					if (PhyAddInx >= 4)
						PhyAddInx -=   4;


					u2Port1PhyAddr = (PHYAddr.Origin | (1 << PhyAddInx));
					u2Port2PhyAddr = (PHYAddr.Origin | (2 << PhyAddInx));
				} else {
					u2Port1PhyAddr = 0xffff;
					u2Port2PhyAddr = 0xffff;
				}

				if (_fgUseModifiedPA == TRUE) { /* for CLI debug mode */

					u2Port1PhyAddr = _u2ModifiedPA;

					u2Port2PhyAddr = _u2ModifiedPA;
				}

				_u2Port1PhyAddr =  u2Port1PhyAddr;
				_u2Port2PhyAddr  = u2Port2PhyAddr;

				if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
					HDMI_LOG(HDMI_LOG_DEBUG, "u2Port1PhyAddr =0x%x", u2Port1PhyAddr);
					HDMI_LOG(HDMI_LOG_DEBUG, "u2Port2PhyAddr =0x%x", u2Port2PhyAddr);
				}
			}

			for (l = 0; l < 5; l++) {
				if (PHY_ADDRESS_START[l] != 0) {
					HDMI_LOG(HDMI_LOG_DEBUG, "1.PHY_ADDRESS_START[%d]=0x%x\n",
						l, PHY_ADDRESS_START[l]);
					j = PHY_ADDRESS_START[l];

					if (j < sizeof(oEDIDBuff)) /*  for klocwork issue */
						oEDIDBuff[j] = (u2Port2PhyAddr >> 8) & 0xff;

					j = j + 1;

					if (j < sizeof(oEDIDBuff)) /*  for klocwork issue */
						oEDIDBuff[j] = u2Port2PhyAddr & 0xff;
				}

			}

			oEDIDBuff[EDID_ADR_CHECK_SUM] = Edid_Calculate_CheckSum(oEDIDBuff);

/*  to do write to sram */

			_u1EDID1CHKSUM = oEDIDBuff[EDID_ADR_CHECK_SUM];
			_u2EDID1PA = u2Port2PhyAddr;
			_u1EDID1PAOFF = Edid.PHYPoint;

			vWriteEDIDBlk1(EEPROM1, oEDIDBuff);

			if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
				/* UTIL_Printf("PHYAddr.Sat = 0x%x\r\n",PHYAddr.Sat); */
				HDMI_LOG(HDMI_LOG_DEBUG, "u2Port2PhyAddr = 0x%x", u2Port2PhyAddr);
			}

			break;

		default:

			break;


		}

		if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) {
			HDMI_LOG(HDMI_LOG_DEBUG, "After  ComposeEdidBlock1 u1EdidNum%d Page1 EDID ===\r\n",
				Edid.Number);

			/* UTIL_Printf("    0  1  2  3  4  5  6  7 - 8  9  A  B  C  D  E  F"); */
			for (j = 0; j < 127; j += 8)

				HDMI_LOG(HDMI_LOG_DEBUG, "Add= %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
				j, oEDIDBuff[j], oEDIDBuff[j + 1], oEDIDBuff[j + 2], oEDIDBuff[j + 3],
				oEDIDBuff[j + 4], oEDIDBuff[j + 5], oEDIDBuff[j + 6], oEDIDBuff[j + 7]);

		}

	}




}


/*==============================================================================*/
void Default_Edid_BL0_BL1_Write(void)
{
	if (_fgEdidEnterToEdition == TRUE) /* kenny 2010/9/1 ,
	if user use CLI "modedid" to edit EDID, then it will not do process by HPD */
		return;

	Default_Edid_BL0_Write();
	Default_Edid_BL1_Write();
}

void vReadRxEDID(UINT8 u1EDID)
{
/* jitao shi to do for sony */
#if 0
	UINT8 Addr;
	UINT32 u4EDIDRamVal;

	if (u1EDID == 0) {
		/* Test */
		for (Addr = 0; Addr < 127; Addr += 1)
			oBlock[Addr] = 0;

		vEnableEDIDDLMODE(TRUE);
		vSetEDIDDLADD(0);

		for (Addr = 0; Addr < 32; Addr++) {
			u4EDIDRamVal = u4ReadEDIDRam();
			oBlock[Addr * 4]     = (u4EDIDRamVal) & 0xFF;
			oBlock[Addr * 4 + 1] = (u4EDIDRamVal >> 8) & 0xFF;
			oBlock[Addr * 4 + 2] = (u4EDIDRamVal >> 16) & 0xFF;
			oBlock[Addr * 4 + 3] = (u4EDIDRamVal >> 24) & 0xFF;
		}

		vEnableEDIDDLMODE(FALSE);

		if (1) { /* fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) */
			HDMI_LOG(HDMI_LOG_DEBUG, "Block 0");

			for (Addr = 0; Addr < 127; Addr += 8)

				HDMI_LOG(HDMI_LOG_DEBUG, "Add= %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
				Addr, oBlock[Addr], oBlock[Addr + 1], oBlock[Addr + 2], oBlock[Addr + 3],
				oBlock[Addr + 4], oBlock[Addr + 5], oBlock[Addr + 6], oBlock[Addr + 7]);

		}

		for (Addr = 0; Addr < 127; Addr += 1)
			oBlock[Addr] = 0;

		vEnableEDIDDLMODE(TRUE);
		vSetEDIDDLADD(32);

		for (Addr = 0; Addr < 32; Addr++) {
			u4EDIDRamVal = u4ReadEDIDRam();
			oBlock[Addr * 4]     = (u4EDIDRamVal) & 0xFF;
			oBlock[Addr * 4 + 1] = (u4EDIDRamVal >> 8) & 0xFF;
			oBlock[Addr * 4 + 2] = (u4EDIDRamVal >> 16) & 0xFF;
			oBlock[Addr * 4 + 3] = (u4EDIDRamVal >> 24) & 0xFF;
		}

		vEnableEDIDDLMODE(FALSE);
		oBlock[127] = _u1EDID0CHKSUM;
		oBlock[_u1EDID0PAOFF] = (_u2EDID0PA >> 0) & 0xFF;
		oBlock[_u1EDID0PAOFF] = (_u2EDID0PA >> 8) & 0xFF;
		oBlock[_u1EDID0PAOFF1] = (_u2EDID0PA1 >> 0) & 0xFF;
		oBlock[_u1EDID0PAOFF1] = (_u2EDID0PA1 >> 8) & 0xFF;

		if (1) { /*fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) */
			HDMI_LOG(HDMI_LOG_DEBUG, "Block 1");

			for (Addr = 0; Addr < 127; Addr += 8)

				HDMI_LOG(HDMI_LOG_DEBUG, "Add= %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
				Addr, oBlock[Addr], oBlock[Addr + 1], oBlock[Addr + 2], oBlock[Addr + 3],
				oBlock[Addr + 4], oBlock[Addr + 5], oBlock[Addr + 6], oBlock[Addr + 7]);

		}


	} else if (u1EDID == 1) {
		/* Test */
		for (Addr = 0; Addr < 127; Addr += 1)
			oBlock[Addr] = 0;

		vEnableEDIDDLMODE(TRUE);
		vSetEDIDDLADD(0);

		for (Addr = 0; Addr < 32; Addr++) {
			u4EDIDRamVal = u4ReadEDIDRam();
			oBlock[Addr * 4]     = (u4EDIDRamVal) & 0xFF;
			oBlock[Addr * 4 + 1] = (u4EDIDRamVal >> 8) & 0xFF;
			oBlock[Addr * 4 + 2] = (u4EDIDRamVal >> 16) & 0xFF;
			oBlock[Addr * 4 + 3] = (u4EDIDRamVal >> 24) & 0xFF;
		}

		vEnableEDIDDLMODE(FALSE);

		if (1) { /*fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) */
			HDMI_LOG(HDMI_LOG_DEBUG, "Block 0");

			for (Addr = 0; Addr < 127; Addr += 8)

				HDMI_LOG(HDMI_LOG_DEBUG, "Add= %2d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
				Addr, oBlock[Addr], oBlock[Addr + 1], oBlock[Addr + 2], oBlock[Addr + 3],
				oBlock[Addr + 4], oBlock[Addr + 5], oBlock[Addr + 6], oBlock[Addr + 7]);

		}

		for (Addr = 0; Addr < 127; Addr += 1)
			oBlock[Addr] = 0;

		vEnableEDIDDLMODE(TRUE);
		vSetEDIDDLADD(32);

		for (Addr = 0; Addr < 32; Addr++) {
			u4EDIDRamVal = u4ReadEDIDRam();
			oBlock[Addr * 4]     = (u4EDIDRamVal) & 0xFF;
			oBlock[Addr * 4 + 1] = (u4EDIDRamVal >> 8) & 0xFF;
			oBlock[Addr * 4 + 2] = (u4EDIDRamVal >> 16) & 0xFF;
			oBlock[Addr * 4 + 3] = (u4EDIDRamVal >> 24) & 0xFF;
		}

		vEnableEDIDDLMODE(FALSE);
		oBlock[127] = _u1EDID1CHKSUM;
		oBlock[_u1EDID1PAOFF] = (_u2EDID1PA >> 0) & 0xFF;
		oBlock[_u1EDID1PAOFF] = (_u2EDID1PA >> 8) & 0xFF;
		oBlock[_u1EDID1PAOFF1] = (_u2EDID1PA1 >> 0) & 0xFF;
		oBlock[_u1EDID1PAOFF1] = (_u2EDID1PA1 >> 8) & 0xFF;

		if (1) { /* fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID)) */
			HDMI_LOG(HDMI_LOG_DEBUG, "Block 1");

			for (Addr = 0; Addr < 127; Addr += 8)

				HDMI_LOG(HDMI_LOG_DEBUG, "Add= %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
				Addr, oBlock[Addr], oBlock[Addr + 1], oBlock[Addr + 2], oBlock[Addr + 3],
				oBlock[Addr + 4], oBlock[Addr + 5], oBlock[Addr + 6], oBlock[Addr + 7]);

		}


	}

#endif
}


void vVerifyWEdidBL0(UINT8 u1Index)
{
#if 0
	UINT8 Addr;

	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
		HDMI_LOG(HDMI_LOG_DEBUG, "Default_Edid_BL0_Write");

	for (Addr = u1Index * 8 ; Addr < (u1Index * 8 + 8); Addr++)
		oBlock[Addr] = Default_Edid_Block0[Addr];


	if (fgEeprom1DataWrite(EDIDID, u1Index * 8, 8, &oBlock[u1Index * 8]) == FALSE) {
		if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Default_Edid_BL0 EEPROM0 Write Addr %d Fail\n", u1Index * 8);
	}

#endif
}



void vVerifyREdidBL0(UINT8 u1Index)
{
#if 0
	UINT8 Addr;

	if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
		HDMI_LOG(HDMI_LOG_DEBUG, "Default_Edid_BL0_Read");

	for (Addr = u1Index * 8 ; Addr < (u1Index * 8 + 8); Addr++)
		oBlock[Addr] = 0;



	if (fgEeprom1DataRead(EDIDID,  u1Index * 8, 8, &oBlock[u1Index * 8]) == FALSE) {
		if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
			HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI RX]Default_Edid_BL0 EEPROM0 Read Addr %d Fail\n", u1Index * 8);
	}

	/* for(Addr = 0; Addr < 8; Addr += 1) */
	{
		if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
			HDMI_LOG(HDMI_LOG_DEBUG, "Add= %d: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
			u1Index * 8, oBlock[u1Index * 8 + Addr], oBlock[u1Index * 8 + Addr + 1],
			oBlock[u1Index * 8 + Addr + 2], oBlock[u1Index * 8 + Addr + 3],
			oBlock[u1Index * 8 + Addr + 4], oBlock[u1Index * 8 + Addr + 5],
			oBlock[u1Index * 8 + Addr + 6], oBlock[u1Index * 8 + Addr + 7]);
	}

#endif
}
void vSetEdidUpdateMode(BOOL fgVideoAndOp, BOOL fgAudioAndOP)
{
	_fgVideoUseAndOperation = fgVideoAndOp;
	_fgAudioUseAndOperation = fgAudioAndOP;
#if CONFIG_DRV_CUSTOM_JXF
	vSetEdidPcm2chOnly(fgAudioAndOP);
#endif
}

void vSetEdidPcm2chOnly(UINT8 u12chPcmOnly)
{
	if (u12chPcmOnly)
		_fg2chPcmOnly = TRUE;
	else
		_fg2chPcmOnly = FALSE;

}

BOOL fgOnlySupportPcm2ch(void)
{
	return _fg2chPcmOnly;

}




void vSetEdidNoDeepColor(UINT8 u1NoDeepColorOnly)
{
	if (u1NoDeepColorOnly)
		_fgNoDeepColor = TRUE;
	else
		_fgNoDeepColor = FALSE;

}

BOOL fgOnlySupportNoDeep(void)
{
	return _fgNoDeepColor;

}

void vShowEdidPhyAddress(void)
{
	/* Printf("[HDMI RX]   _u2Port1PhyAddr = 0x%x,  _u2Port2PhyAddr = 0x%x", _u2Port1PhyAddr, _u2Port2PhyAddr); */
	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]EDID1 PA = %d. %d. %d. %d", (_u2Port1PhyAddr >> 12) & 0x0f,
	(_u2Port1PhyAddr >> 8) & 0x0f, (_u2Port1PhyAddr >> 4) & 0x0f, _u2Port1PhyAddr & 0x0f);
	HDMI_LOG(HDMI_LOG_INFO, "[HDMI RX]EDID2 PA = %d. %d. %d. %d", (_u2Port2PhyAddr >> 12) & 0x0f,
	(_u2Port2PhyAddr >> 8) & 0x0f, (_u2Port2PhyAddr >> 4) & 0x0f, _u2Port2PhyAddr & 0x0f);

}

void vModifyEdidPa(UINT16 u2PA)
{
	_fgUseModifiedPA = TRUE;
	_u2ModifiedPA = u2PA;

}

void vRecoverEdidPa(void)
{
	_fgUseModifiedPA = FALSE;
	_u2ModifiedPA = 0;

}

void vSelectEdidToEdit(UINT8 u1EdidNum, UINT8 u1Block)
{
/* jitao shi to do for sony */
#if 0
	UINT8 Addr;
	UINT32 u4EDIDRamVal = 0;

	_u1ModifiedEdidNum =  u1EdidNum;
	_u1ModifiedEdidBlock =    u1Block;

	if (u1EdidNum == 0) {
		/* Test */
		for (Addr = 0; Addr < 127; Addr += 1)
			modBlock[Addr] = 0;


		if (u1Block == 0) {
			vEnableEDIDDLMODE(TRUE);
			vSetEDIDDLADD(0);

			for (Addr = 0; Addr < 32; Addr++) {
				u4EDIDRamVal = u4ReadEDIDRam();
				modBlock[Addr * 4]     = (u4EDIDRamVal) & 0xFF;
				modBlock[Addr * 4 + 1] = (u4EDIDRamVal >> 8) & 0xFF;
				modBlock[Addr * 4 + 2] = (u4EDIDRamVal >> 16) & 0xFF;
				modBlock[Addr * 4 + 3] = (u4EDIDRamVal >> 24) & 0xFF;
			}

			vEnableEDIDDLMODE(FALSE);
		} else {
			vEnableEDIDDLMODE(TRUE);
			vSetEDIDDLADD(32);

			for (Addr = 0; Addr < 32; Addr++) {
				u4EDIDRamVal = u4ReadEDIDRam();
				modBlock[Addr * 4]     = (u4EDIDRamVal) & 0xFF;
				modBlock[Addr * 4 + 1] = (u4EDIDRamVal >> 8) & 0xFF;
				modBlock[Addr * 4 + 2] = (u4EDIDRamVal >> 16) & 0xFF;
				modBlock[Addr * 4 + 3] = (u4EDIDRamVal >> 24) & 0xFF;
			}

			vEnableEDIDDLMODE(FALSE);
			modBlock[127] = _u1EDID0CHKSUM;
			modBlock[_u1EDID0PAOFF] = (_u2EDID0PA >> 0) & 0xFF;
			modBlock[_u1EDID0PAOFF] = (_u2EDID0PA >> 8) & 0xFF;
			modBlock[_u1EDID0PAOFF1] = (_u2EDID0PA1 >> 0) & 0xFF;
			modBlock[_u1EDID0PAOFF1] = (_u2EDID0PA1 >> 8) & 0xFF;

		}

		if (u1Block == 0)
			HDMI_LOG(HDMI_LOG_DEBUG, "EEPROM 1, Block 0");
		else
			HDMI_LOG(HDMI_LOG_DEBUG, "EEPROM 1, Block 1");

		for (Addr = 0; Addr < 127; Addr += 8)

			HDMI_LOG(HDMI_LOG_DEBUG, "Add= %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
			Addr, modBlock[Addr], modBlock[Addr + 1], modBlock[Addr + 2], modBlock[Addr + 3],
			modBlock[Addr + 4], modBlock[Addr + 5], modBlock[Addr + 6], modBlock[Addr + 7]);

	} else if (u1EdidNum == 1) {
		/* Test */
		for (Addr = 0; Addr < 127; Addr += 1)
			modBlock[Addr] = 0;

		if (u1Block == 0) {
			vEnableEDIDDLMODE(TRUE);
			vSetEDIDDLADD(0);

			for (Addr = 0; Addr < 32; Addr++) {
				u4EDIDRamVal = u4ReadEDIDRam();
				modBlock[Addr * 4]     = (u4EDIDRamVal) & 0xFF;
				modBlock[Addr * 4 + 1] = (u4EDIDRamVal >> 8) & 0xFF;
				modBlock[Addr * 4 + 2] = (u4EDIDRamVal >> 16) & 0xFF;
				modBlock[Addr * 4 + 3] = (u4EDIDRamVal >> 24) & 0xFF;
			}

			vEnableEDIDDLMODE(FALSE);
		} else {
			vEnableEDIDDLMODE(TRUE);
			vSetEDIDDLADD(32);

			for (Addr = 0; Addr < 32; Addr++) {
				u4EDIDRamVal = u4ReadEDIDRam();
				modBlock[Addr * 4]     = (u4EDIDRamVal) & 0xFF;
				modBlock[Addr * 4 + 1] = (u4EDIDRamVal >> 8) & 0xFF;
				modBlock[Addr * 4 + 2] = (u4EDIDRamVal >> 16) & 0xFF;
				modBlock[Addr * 4 + 3] = (u4EDIDRamVal >> 24) & 0xFF;
			}

			vEnableEDIDDLMODE(FALSE);
			modBlock[127] = _u1EDID1CHKSUM;
			modBlock[_u1EDID1PAOFF] = (_u2EDID1PA >> 0) & 0xFF;
			modBlock[_u1EDID1PAOFF] = (_u2EDID1PA >> 8) & 0xFF;
			modBlock[_u1EDID1PAOFF1] = (_u2EDID1PA1 >> 0) & 0xFF;
			modBlock[_u1EDID1PAOFF1] = (_u2EDID1PA1 >> 8) & 0xFF;
		}


		if (u1Block == 0)
			HDMI_LOG(HDMI_LOG_DEBUG, "EEPROM 2, Block 0");
		else
			HDMI_LOG(HDMI_LOG_DEBUG, "EEPROM 2, Block 1");

		for (Addr = 0; Addr < 127; Addr += 8)

			HDMI_LOG(HDMI_LOG_DEBUG, "Add= %2d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
			Addr, modBlock[Addr], modBlock[Addr + 1], modBlock[Addr + 2], modBlock[Addr + 3],
			modBlock[Addr + 4], modBlock[Addr + 5], modBlock[Addr + 6], modBlock[Addr + 7]);



	}

	_fgEdidEnterToEdition = TRUE;
#endif
}

void vShowEditionEdidBlock(void)
{
	UINT8 Addr;

	if ((_u1ModifiedEdidBlock == 0xff) || (_u1ModifiedEdidNum == 0xff)) {
		HDMI_LOG(HDMI_LOG_INFO, "Please use modedid s eeprom# block# to edit\n");
		return;
	}


	HDMI_LOG(HDMI_LOG_DEBUG, "EEPROM %d, Block %d", _u1ModifiedEdidNum + 1, _u1ModifiedEdidBlock);

	for (Addr = 0; Addr < 127; Addr += 8)
		HDMI_LOG(HDMI_LOG_DEBUG, "Add= %2d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x",
		Addr, modBlock[Addr], modBlock[Addr + 1], modBlock[Addr + 2], modBlock[Addr + 3],
		modBlock[Addr + 4], modBlock[Addr + 5], modBlock[Addr + 6], modBlock[Addr + 7]);



}



void vModifyEditionEdid(UINT8 Addr, UINT8 u1Value)
{


	if ((_u1ModifiedEdidBlock == 0xff) || (_u1ModifiedEdidNum == 0xff)) {
		HDMI_LOG(HDMI_LOG_INFO, "Please use modedid s eeprom# block# to edit");
		return;
	}

	HDMI_LOG(HDMI_LOG_DEBUG, "EEPROM %d, Block %d", _u1ModifiedEdidNum + 1, _u1ModifiedEdidBlock);

	modBlock[Addr] = u1Value;

	modBlock[EDID_ADR_CHECK_SUM] = Edid_Calculate_CheckSum(&modBlock[0]);

	HDMI_LOG(HDMI_LOG_DEBUG, "modBlock[%d]= 0x%2x", Addr, modBlock[Addr]);
	HDMI_LOG(HDMI_LOG_DEBUG, "chech sum = 0x%2x", modBlock[EDID_ADR_CHECK_SUM]);
}


void vWriteEditionEdidToEEPROM(void)
{
/* jitao shi to do for sony */
#if 0
	UINT8 Addr;

	if ((_u1ModifiedEdidBlock == 0xff) || (_u1ModifiedEdidNum == 0xff)) {
		HDMI_LOG(HDMI_LOG_DEBUG, "Please use modedid s eeprom# block# to edit\n");
		return;
	}

	HDMI_LOG(HDMI_LOG_DEBUG, "Write EEPROM#%d , Block#%d Start\n", _u1ModifiedEdidNum + 1, _u1ModifiedEdidBlock);


	if (_u1ModifiedEdidNum == EEPROM0) {

		if (_u1ModifiedEdidBlock == 0) {
			vEnableEDIDDLMODE(TRUE);
			vSetEDIDDLADD(0);

			for (Addr = 0; Addr < 32; Addr++) {
				vWriteEDIDRam(modBlock[Addr * 4] | (modBlock[Addr * 4 + 1] << 8) |
				(modBlock[Addr * 4 + 2] << 16) | (modBlock[Addr * 4 + 3] << 24));
				/* address 255 is invalid */
			}

			vEnableEDIDDLMODE(FALSE);
		} else if (_u1ModifiedEdidBlock == 1) {
			vEnableEDIDDLMODE(TRUE);
			vSetEDIDDLADD(32);

			for (Addr = 0; Addr < 32; Addr++) {
				vWriteEDIDRam(modBlock[Addr * 4] | (modBlock[Addr * 4 + 1] << 8) |
				(modBlock[Addr * 4 + 2] << 16) | (modBlock[Addr * 4 + 3] << 24));
				/* address 255 is invalid */
			}

			vEnableEDIDDLMODE(FALSE);
			vWriteEDIDCHKSUM(EDIDDEV0, modBlock[127]);

			if (_u1EDID0PAOFF < 127 && _u1EDID0PAOFF1 < 127)
				vWriteEDIDPA(EDIDDEV0,
				(modBlock[_u1EDID0PAOFF] << 8) | (modBlock[_u1EDID0PAOFF + 1] << 0),
				_u1EDID0PAOFF + 0x80,
				(modBlock[_u1EDID0PAOFF1] << 8) | (modBlock[_u1EDID0PAOFF1 + 1] << 0),
				_u1EDID0PAOFF1 + 0x80);
		}

	} else if (_u1ModifiedEdidNum == EEPROM1) {
		if (_u1ModifiedEdidBlock == 0) {
			vEnableEDIDDLMODE(TRUE);
			vSetEDIDDLADD(0);

			for (Addr = 0; Addr < 32; Addr++) {
				vWriteEDIDRam(modBlock[Addr * 4] | (modBlock[Addr * 4 + 1] << 8) |
				(modBlock[Addr * 4 + 2] << 16) | (modBlock[Addr * 4 + 3] << 24));
				/* address 255 is invalid */
			}

			vEnableEDIDDLMODE(FALSE);
		} else if (_u1ModifiedEdidBlock == 1) {
			vEnableEDIDDLMODE(TRUE);
			vSetEDIDDLADD(32);

			for (Addr = 0; Addr < 32; Addr++) {
				vWriteEDIDRam(modBlock[Addr * 4] | (modBlock[Addr * 4 + 1] << 8) |
				(modBlock[Addr * 4 + 2] << 16) | (modBlock[Addr * 4 + 3] << 24));
				/* address 255 is invalid */
			}

			vEnableEDIDDLMODE(FALSE);
			vWriteEDIDCHKSUM(EDIDDEV1, modBlock[127]);

			if (_u1EDID1PAOFF < 127 && _u1EDID1PAOFF1 < 127)
				vWriteEDIDPA(EDIDDEV1,
				(modBlock[_u1EDID1PAOFF] << 8) | (modBlock[_u1EDID1PAOFF + 1] << 0),
				_u1EDID1PAOFF + 0x80,
				(modBlock[_u1EDID1PAOFF1] << 8) | (modBlock[_u1EDID1PAOFF1 + 1] << 0),
				_u1EDID1PAOFF1 + 0x80);
		}
	}

	_fgEdidEnterToEdition = FALSE;
	_u1ModifiedEdidBlock = 0xff;
	_u1ModifiedEdidNum = 0xff;

	HDMI_LOG(HDMI_LOG_INFO, "Write EEPROM#%d , Block#%d OK\n", _u1ModifiedEdidNum + 1, _u1ModifiedEdidBlock);
#endif
}


void vQuitEdidEdition(void)
{
	_fgEdidEnterToEdition = FALSE;
	_u1ModifiedEdidBlock = 0xff;
	_u1ModifiedEdidNum = 0xff;
}


void vEDIDCreateBlock1(void)
{
	/* UINT8 i,j,l,PhyAddInx; */
	/* UINT16 u2Port1PhyAddr = 0,u2Port2PhyAddr = 0; */
	UINT8 Addr, i;/* , j; */
	UINT8 u1EdidNum, u1PAHigh, u1PALow;
	UINT8 u1PhyAddInx;

	UINT16 u2Port1PhyAddr, u2Port2PhyAddr;
	/* UINT8 DataBlkLen  = 0; */


	HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI Rx EDID] vEDIDCreateBlock1");

	if (_fgAudioUseAndOperation == FALSE) {
		for (i = 0; i < 128; i++) {
			rEDIDBuff[i] = 0;
			oEDIDBuff[i] = 0;
		}


		{
/****************************************/
/*  Block 1 */
/*  Hdmi.bLatency = true; */
/*  Hdmi.ALatency = 0; */

			HDMI_LOG(HDMI_LOG_DEBUG, "Default_Edid_BL1_Write");


			for (Addr = 0; Addr < 4; Addr++)
				oEDIDBuff[Addr] = Default_Edid_Block1_Header[Addr];

			if (_fgAudioUseAndOperation == FALSE) /* Audio output is HDMI+Speaket or Speaker */
				oEDIDBuff[1] = 3;

			if (_HdmiSinkAvCap.b_sink_support_hdmi_mode == FALSE) { /* DVI */
				oEDIDBuff[3] = oEDIDBuff[3] & (0x8f); /* clear YcbCr444 and Ycbcr422 */

				if (_fgAudioUseAndOperation == FALSE) /* Audio output is HDMI+Speaket or Speaker */
					oEDIDBuff[3] = oEDIDBuff[3] | (1 << 6); /* Set basic audio */
			}




			/*
			for(i=0; i<EDID_VIDEO_BLOCK_LEN ; i++, Addr ++)
			oBlock[Addr] = Video_Data_Block[i];
			*/
			if (_fgVideoUseAndOperation)
				ComposeVideoBlock(1, _fgAudioUseAndOperation);
			else
				ComposeVideoBlock(0, _fgAudioUseAndOperation);

			if ((Compose_Video_Data_Block[0] & 0x1f) != 0) { /* SVD exist */
				/* DataBlkLen = DataBlkLen+(Compose_Video_Data_Block[0]&0x1f)+1;
				Add Video block length */
				for (i = 0; i < ((Compose_Video_Data_Block[0] & 0x1f) + 1); i++,  Addr++)
					oEDIDBuff[Addr] = Compose_Video_Data_Block[i];

				/* for(i=0; i< ((Compose_Video_Data_Block[0]&0x1f)+1); i++,  oCount++) */
				/* (poBlock+oCount) = Compose_Video_Data_Block[i]; */

			}

			/*
			if(fgOnlySupportPcm2ch() == TRUE)
			{
			  for(i=0; i<EDID_AUDIO_BLOCK_LEN ; i++)
			  Audio_Data_Block[i]=0;

			  for(i=0; i< sizeof(Audio_Data_Block_2CH_ONLY) ; i++)
			  Audio_Data_Block[i]= Audio_Data_Block_2CH_ONLY[i];

			}
			*/


/* for(i=0; i<EDID_AUDIO_BLOCK_LEN ; i++, Addr ++)
for(i=0; i< (Audio_Data_Block[0]&0x1f)+1 ; i++, Addr ++)
oBlock[Addr] = Audio_Data_Block[i]; */
			if (fgOnlySupportPcm2ch() == TRUE) {
				for (i = 0; (i < EDID_AUDIO_2CH_PCM_ONLY_BLOCK_LEN) && (Addr < 128) ; i++, Addr++)
					oEDIDBuff[Addr] = Audio_Data_Block_2CH_ONLY[i];
			} else {
				for (i = 0; (i < EDID_AUDIO_BLOCK_LEN) && (Addr < 128); i++, Addr++)
					oEDIDBuff[Addr] = Audio_Data_Block[i];
			}

			/*
			if(fgOnlySupportPcm2ch() == TRUE)
			{
			  for(i=0; i<EDID_SPEAKER_BLOCK_LEN ; i++)
			    Speaker_Data_Block[i] = Speaker_Data_Block_2CH_ONLY[i];
			}
			*/

			if (fgOnlySupportPcm2ch() == TRUE) {
				for (i = 0; (i < EDID_SPEAKER_BLOCK_LEN) && (Addr < 128); i++, Addr++)
					oEDIDBuff[Addr] = Speaker_Data_Block_2CH_ONLY[i];
			} else {
				for (i = 0; (i < EDID_SPEAKER_BLOCK_LEN) && (Addr < 128); i++, Addr++)
					oEDIDBuff[Addr] = Speaker_Data_Block[i];
			}

			u1PhyAddInx = Addr + 4;

			for (i = 0; i < EDID_VENDOR_BLOCK_MINI_LEN; i++)
				Vendor_Data_Block_Full[i] = Vendor_Data_Block_Mini[i];

	for (i = 0; i < EDID_VENDOR_BLOCK_MINI_LEN ; i++, Addr++) {
		if (Addr < 128) {
			if ((i == 4) || (i == 5)) { /* PA start address */
				if (_fgUseModifiedPA == TRUE) {
					if (i == 4)
						oEDIDBuff[Addr] = (_u2ModifiedPA >> 8) & 0xff;
					if (i == 5)
						oEDIDBuff[Addr] = _u2ModifiedPA & 0xff;
				} else
					oEDIDBuff[Addr] = Vendor_Data_Block_Full[i];
			} else
				oEDIDBuff[Addr] = Vendor_Data_Block_Full[i];
		}
	}

			oEDIDBuff[2] = Addr;/* Update data offset */


			for (; Addr < 128; Addr++)
				oEDIDBuff[Addr] =  0;


			u2Port1PhyAddr = 0x1100;
			u2Port2PhyAddr = 0x1200;


			for (u1EdidNum = EEPROM0; u1EdidNum <= EEPROM1; u1EdidNum++) {
				switch (u1EdidNum) {
				case  EEPROM0:
					u1PAHigh = (UINT8)(u2Port1PhyAddr >> 8);
					u1PALow = (UINT8)(u2Port1PhyAddr & 0xFF);

					if ((u1PhyAddInx + 1) < sizeof(oEDIDBuff)) { /* for klocwork issue */
						oEDIDBuff[u1PhyAddInx] = u1PAHigh;
						oEDIDBuff[u1PhyAddInx + 1] = u1PALow;
					}

					_u2Port1PhyAddr = u2Port1PhyAddr;

					if (1) { /* fgIsTxDetectHotPlugIn() == TRUE) //081103 */
					if ((u1PhyAddInx + 2) < sizeof(oEDIDBuff)) { /* for klocwork issue */
						oEDIDBuff[u1PhyAddInx + 2] &= 0x80;   /* Del eDeep Color Info */
					}
					}


					oEDIDBuff[EDID_ADR_CHECK_SUM] =
					Edid_Calculate_CheckSum(&oEDIDBuff[0]); /* 081103 */
					_u1EDID0CHKSUM = oBlock[EDID_ADR_CHECK_SUM];
					_u2EDID0PA = u2Port1PhyAddr;
					_u1EDID0PAOFF = u1PhyAddInx;
					vWriteEDIDBlk1(EEPROM0, oEDIDBuff);

					break;

				case  EEPROM1:
					u1PAHigh = (UINT8)(u2Port2PhyAddr >> 8);
					u1PALow = (UINT8)(u2Port2PhyAddr & 0xFF);

					if ((u1PhyAddInx + 1) < sizeof(oEDIDBuff)) { /* for klocwork issue */
						oEDIDBuff[u1PhyAddInx] = u1PAHigh;
						oEDIDBuff[u1PhyAddInx + 1] = u1PALow;
					}

					_u2Port2PhyAddr = u2Port2PhyAddr;

					if (1) { /* fgIsTxDetectHotPlugIn() == TRUE) 081103 */
					if ((u1PhyAddInx + 2) < sizeof(oEDIDBuff)) /* for klocwork issue */
						oEDIDBuff[u1PhyAddInx + 2] &= 0x80; /* Del eDeep Color Info */
					}

					oEDIDBuff[EDID_ADR_CHECK_SUM] =
						Edid_Calculate_CheckSum(&oEDIDBuff[0]);   /* 081103 */
					_u1EDID1CHKSUM = oEDIDBuff[EDID_ADR_CHECK_SUM];
					_u2EDID1PA = u2Port2PhyAddr;
					_u1EDID1PAOFF = u1PhyAddInx;
					vWriteEDIDBlk1(EEPROM1, oEDIDBuff);
					break;

				default:
					break;
				} /* switch(u1EdidNum) */

			} /* for(u1EdidNum=EEPROM0; u1EdidNum <= EEPROM1; u1EdidNum++) */

			Edid.bBlock1Err = TRUE;
			Edid.bDownDvi = TRUE;

			PHYAddr.Origin = 0x0000;
			PHYAddr.Dvd = 0x1000;
			PHYAddr.Sat = 0x2000;
			PHYAddr.Tv = 0x3000;
			PHYAddr.Else = 0x4000;

			Edid.PHYLevel = 1;

		}

	} else {
		for (i = 0; i < 128; i++)
			oEDIDBuff[i] = 0;

		_u1EDID0CHKSUM = 0;
		_u1EDID1CHKSUM = 0;
		_u2EDID0PA = 0;
		_u2EDID1PA = 0;
		_u1EDID0PAOFF = 0;
		_u1EDID1PAOFF = 0;
		vWriteEDIDBlk1(EEPROM0, oEDIDBuff);
		vWriteEDIDBlk1(EEPROM1, oEDIDBuff);
	}
}

#ifdef DRV_SUPPORT_WRITE_EDID_DIRRECT_HDMI_RX
#if DRV_SUPPORT_WRITE_EDID_DIRRECT_HDMI_RX
UINT8 *pbEDID0 = NULL;
UINT8 *pbEDID1 = NULL;
#endif
#endif
void vWriteEDIDBlk0(UINT8 u1EDIDNo, UINT8 *poBlock)
{
	UINT8 Addr, u1Cnt;

#ifdef DRV_SUPPORT_WRITE_EDID_DIRRECT_HDMI_RX
#if DRV_SUPPORT_WRITE_EDID_DIRRECT_HDMI_RX

	if ((pbEDID0  ==  NULL) || (pbEDID1 == NULL)) {
		HDMI_LOG(HDMI_LOG_WARN, "[HDMI RX]EDID0 data not ready\n");
		return;
	}

	if (u1EDIDNo == EEPROM0)
		poBlock = pbEDID0;
	else
		poBlock = pbEDID1;

#endif
#endif

#ifdef EDID_SUPPORT_BYPASS_TX_EDID
#if EDID_SUPPORT_BYPASS_TX_EDID

	if (vGetTxPortEdid(poBlock, 0, 128) == FALSE)
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI Rx] BLK0 Not BYPASS_TX_EDID");
	else
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI Rx] BLK0 BYPASS_TX_EDID");

#endif
#endif


	if (u1EDIDNo == EEPROM0) {
		for (u1Cnt = 0; u1Cnt < 128; u1Cnt++)
			u1HDMIIN1EDID[u1Cnt] = *(poBlock + u1Cnt);
	}

	if (u1EDIDNo == EEPROM1) {
		for (u1Cnt = 0; u1Cnt < 128; u1Cnt++)
			u1HDMIIN2EDID[u1Cnt] = *(poBlock + u1Cnt);
	}

	for (Addr = 0; Addr < 127; Addr += 8)

		HDMI_LOG(HDMI_LOG_DEBUG, "Add= %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",
		Addr, poBlock[Addr], poBlock[Addr + 1], poBlock[Addr + 2], poBlock[Addr + 3],
		poBlock[Addr + 4], poBlock[Addr + 5], poBlock[Addr + 6], poBlock[Addr + 7]);


#if (EDID_SUPPORT_MODE == INTERAL_MODE)

	if ((u1EDIDNo == EEPROM0) || (u1EDIDNo == EEPROM1)) {
		vEnableEDIDDLMODE(TRUE);
		vSetEDIDDLADD(0);

		for (Addr = 0; Addr < 32; Addr++) {
			vWriteEDIDRam(poBlock[Addr * 4] | (poBlock[Addr * 4 + 1] << 8) |
			(poBlock[Addr * 4 + 2] << 16) | (poBlock[Addr * 4 + 3] << 24));
			/* address 255 is invalid */
		}

		vEnableEDIDDLMODE(FALSE);

		/* Delay5MS(2); */
	}

#elif (EDID_SUPPORT_MODE == TWO_EXT_MODE)
	vHDMIRxDDCChgToMaster(TRUE);

	if (u1EDIDNo == EEPROM0) {

		for (Addr = 0; Addr < 127; Addr += 8) {
			if (fgEeprom1DataWrite(EDIDID, Addr, 8, &poBlock[Addr]) == FALSE) {
				if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
					HDMI_LOG(HDMI_LOG_WARN,
					"[HDMI RX]Default_Edid_BL0 EEPROM0 Write Addr %d Fail\n", Addr);
			}

			Delay5MS(2);

		}
	} else if (u1EDIDNo == EEPROM1) {

		for (Addr = 0; Addr < 127; Addr += 8) {
			if (fgEeprom2DataWrite(EDIDID, Addr, 8, &poBlock[Addr]) == FALSE) {
				if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
					HDMI_LOG(HDMI_LOG_WARN,
					"[HDMI RX]Default_Edid_BL0 EEPROM1 Write Addr %d Fail\n", Addr);
			}

			Delay5MS(2);

		}
	}

	vHDMIRxDDCChgToMaster(FALSE);
#elif (EDID_SUPPORT_MODE == MIX_MODE)

	if (u1EDIDNo == EEPROM0) {
		vEnableEDIDDLMODE(TRUE);
		vSetEDIDDLADD(0);

		for (Addr = 0; Addr < 32; Addr++) {
			vWriteEDIDRam(poBlock[Addr * 4] | (poBlock[Addr * 4 + 1] << 8) |
			(poBlock[Addr * 4 + 2] << 16) | (poBlock[Addr * 4 + 3] << 24));
			/* address 255 is invalid */
		}

		vEnableEDIDDLMODE(FALSE);

	} else if (u1EDIDNo == EEPROM1) {
		vHDMIRxDDCChgToMaster(TRUE);
		msleep(1);

		for (Addr = 0; Addr < 127; Addr += 8) {
			if (fgEeprom2DataWrite(EDIDID, Addr, 8, &poBlock[Addr]) == FALSE) {
				if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
					HDMI_LOG(HDMI_LOG_WARN,
					"[HDMI RX]Default_Edid_BL0 EEPROM1 Write Addr %d Fail\n", Addr);
			}

			Delay5MS(5);

		}

		vHDMIRxDDCChgToMaster(FALSE);
	}

#endif



}

void vWriteEDIDBlk1(UINT8 u1EDIDNo, UINT8 *poBlock)
{

	UINT8 Addr, u1Cnt;
#ifdef DRV_SUPPORT_WRITE_EDID_DIRRECT_HDMI_RX
#if DRV_SUPPORT_WRITE_EDID_DIRRECT_HDMI_RX

	if ((pbEDID0  ==  NULL) || (pbEDID1 == NULL)) {
		HDMI_LOG(HDMI_LOG_WARN, "[HDMI RX]EDID0 data not ready\n");
		return;
	}

	if (u1EDIDNo == EEPROM0)
		poBlock = pbEDID0 + 128;
	else
		poBlock = pbEDID1 + 128;

#endif
#endif

#ifdef EDID_SUPPORT_BYPASS_TX_EDID
#if EDID_SUPPORT_BYPASS_TX_EDID

	if (vGetTxPortEdid(poBlock, 1, 128) == FALSE)
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI Rx] BLK1 Not BYPASS_TX_EDID");
	else {
		HDMI_LOG(HDMI_LOG_DEBUG, "[HDMI Rx] BLK1 BYPASS_TX_EDID");

		if (u1EDIDNo == EEPROM0) {
			_u1EDID0CHKSUM = *(poBlock + 127);
			_u2EDID0PA = (*(poBlock + 1)) | (((UINT16)(*(poBlock + 0))) << 8);
			_u1EDID0PAOFF = 0;
		} else if (u1EDIDNo == EEPROM1) {
			_u1EDID1CHKSUM = *(poBlock + 127);
			_u2EDID1PA = (*(poBlock + 1)) | ((UINT16)(*(poBlock + 0)) << 8);
			_u1EDID1PAOFF = 0;
		}
	}

#endif
#endif

	if (u1EDIDNo == EEPROM0) {
		for (u1Cnt = 0; u1Cnt < 128; u1Cnt++)
			u1HDMIIN1EDID[128 + u1Cnt] = *(poBlock + u1Cnt);
	}

	if (u1EDIDNo == EEPROM1) {
		for (u1Cnt = 0; u1Cnt < 128; u1Cnt++)
			u1HDMIIN2EDID[128 + u1Cnt] = *(poBlock + u1Cnt);
	}


	for (Addr = 0; Addr < 127; Addr += 8)

		HDMI_LOG(HDMI_LOG_DEBUG, "Add= %3d: 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",
		Addr, poBlock[Addr], poBlock[Addr + 1], poBlock[Addr + 2], poBlock[Addr + 3],
		poBlock[Addr + 4], poBlock[Addr + 5], poBlock[Addr + 6], poBlock[Addr + 7]);


#if (EDID_SUPPORT_MODE == INTERAL_MODE)

	if ((u1EDIDNo == EEPROM0) || (u1EDIDNo == EEPROM1)) {
		vEnableEDIDDLMODE(TRUE);
		vSetEDIDDLADD(32);

		for (Addr = 0; Addr < 32; Addr++) {
			vWriteEDIDRam(poBlock[Addr * 4] | (poBlock[Addr * 4 + 1] << 8) |
			(poBlock[Addr * 4 + 2] << 16) | (poBlock[Addr * 4 + 3] << 24));
			/* address 255 is invalid */
		}

		vEnableEDIDDLMODE(FALSE);

		if (u1EDIDNo == EEPROM0) {
			vWriteEDIDCHKSUM(EDIDDEV0, _u1EDID0CHKSUM);
			vWriteEDIDPA(EDIDDEV0, _u2EDID0PA, _u1EDID0PAOFF + 0x80);
		} else if (u1EDIDNo == EEPROM1) {
			vWriteEDIDCHKSUM(EDIDDEV1, _u1EDID1CHKSUM);
			vWriteEDIDPA(EDIDDEV1, _u2EDID1PA, _u1EDID1PAOFF + 0x80);
		}
	}

#elif (EDID_SUPPORT_MODE == TWO_EXT_MODE)

	vHDMIRxDDCChgToMaster(TRUE);

	if (u1EDIDNo == EEPROM0) {

		for (Addr = 0; Addr < 127; Addr += 8) {
			if (fgEeprom1DataWrite(EDIDID, 0x80 + Addr, 8, &poBlock[Addr]) == FALSE) {
				if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
					HDMI_LOG(HDMI_LOG_WARN,
					"[HDMI RX]Default_Edid_BL1 EEPROM0 Write Addr %d Fail\n", Addr);
			}

			Delay5MS(2);
		}
	} else if (u1EDIDNo == EEPROM1) {

		for (Addr = 0; Addr < 127; Addr += 8) {
			if (fgEeprom2DataWrite(EDIDID, 0x80 + Addr, 8, &poBlock[Addr]) == FALSE) {
				if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
					HDMI_LOG(HDMI_LOG_WARN,
					"[HDMI RX]Default_Edid_BL1 EEPROM1 Write Addr %d Fail\n", Addr);
			}

			Delay5MS(2);

		}
	}

	vHDMIRxDDCChgToMaster(FALSE);
#elif (EDID_SUPPORT_MODE == MIX_MODE)

	if (u1EDIDNo == EEPROM0) {
		vEnableEDIDDLMODE(TRUE);
		vSetEDIDDLADD(32);

		for (Addr = 0; Addr < 32; Addr++) {
			vWriteEDIDRam(poBlock[Addr * 4] | (poBlock[Addr * 4 + 1] << 8) |
			(poBlock[Addr * 4 + 2] << 16) | (poBlock[Addr * 4 + 3] << 24));
			/* address 255 is invalid */
		}

		vEnableEDIDDLMODE(FALSE);

		if (u1EDIDNo == EEPROM0) {
			vWriteEDIDCHKSUM(EDIDDEV0, _u1EDID0CHKSUM);
			vWriteEDIDPA(EDIDDEV0, _u2EDID0PA, _u1EDID0PAOFF + 0x80);
		} else if (u1EDIDNo == EEPROM1) {
			vWriteEDIDCHKSUM(EDIDDEV1, _u1EDID1CHKSUM);
			vWriteEDIDPA(EDIDDEV1, _u2EDID1PA, _u1EDID1PAOFF + 0x80);
		}
	} else if (u1EDIDNo == EEPROM1) {
		vHDMIRxDDCChgToMaster(TRUE);
		msleep(1);

		for (Addr = 0; Addr < 127; Addr += 8) {
			if (fgEeprom2DataWrite(EDIDID, 0x80 + Addr, 8, &poBlock[Addr]) == FALSE) {
				if (fgIsHdmiRxDebug(HDMI_RX_DEBUG_EDID))
					HDMI_LOG(HDMI_LOG_WARN,
					"[HDMI RX]Default_Edid_BL1 EEPROM1 Write Addr %d Fail\n", Addr);
			}

			Delay5MS(5);

		}

		vHDMIRxDDCChgToMaster(FALSE);
	}

#endif

}

UINT8 bGetCbusEDID(UINT8 uOffset)
{
	UINT8 bGetData;

	bGetData = u1HDMIINEDID[uOffset];
	return bGetData;
}


#endif /* #if (DRV_SUPPORT_HDMI_RX) */
