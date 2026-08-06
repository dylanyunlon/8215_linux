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
       
#ifndef _SCL_HW_H_
#define _SCL_HW_H_

#include "x_hal_io.h"
#include "x_hal_ic.h"

#ifdef __linux__
#ifndef IO_BASE_VA
#define IO_BASE_VA                      0xFD000000
#endif
#else
#ifndef IO_BASE_VA
#define IO_BASE_VA                      0xA0000000
#endif
#endif

#define SCL_HAL_SCL_REG                 (IO_BASE_VA + 0xA4500)
#define SCL_HAL_FMT_REG                 (IO_BASE_VA + 0xA4600)

#define SCL_HAL_SCL_REG_NUM             (0x100 >> 2)
#define SCL_HAL_FMT_REG_NUM             (0x100 >> 2)

#define SCL_HAL_REG_MODE_NULL           0
#define SCL_HAL_REG_MODE_READ           (1 << 0)
#define SCL_HAL_REG_MODE_WRITE          (1 << 1)

#define FPD_HAL_CONFIG_REG              (IO_BASE_VA + 0xA48E0)
#define SCL_CLK_SEL_MSK                 (0xC)
#define VSCL_CLK_SEL                    (0x1 << 3) /* 1 - Output clock, 0 - Input clock*/
#define HSCL_CLK_SEL                    (0x1 << 2) /* 1 - Output clock, 0 - Input clock*/

#define CP_ENABLE_HW_REG                (IO_BASE_VA + 0x1F080)
#define CP_ENABLE_MSK                   (0x1)

typedef struct _SCL_HAL_SCL_FIELD_T {
	/* DWORD - 000 */
	__u32                                  : 32;

	/* DWORD - 004 */
	__u32                                  : 32;

	/* DWORD - 008 */
	__u32                                  : 32;

	/* DWORD - 00C */
	__u32                                  : 32;

	/* DWORD - 010 */
	__u32                                  : 8;
	__u32      u4DW_NEED                   : 9;
	__u32                                  : 15;

	/* DWORD - 014 */
	__u32      u4VSCALE                    : 22;
	__u32                                  : 10;

	/* DWORD - 018 */
	__u32      u4DENOMINATOR               : 11;
	__u32                                  : 5;

	__u32      u4YSSLTT98                  : 2;
	__u32      u4YSSLBB98                  : 2;
	__u32      u4YSSLBT98                  : 2;
	__u32      u4YSSLTB98                  : 2;
	__u32      u4CSSLTT98                  : 2;
	__u32      u4CSSLBB98                  : 2;
	__u32      u4CSSLBT98                  : 2;
	__u32      u4CSSLTB98                  : 2;

	/* DWORD - 01C */
	__u32      fgSCL12OUT                  : 1;
	__u32      fgNOSCALE                   : 1;
	__u32      fgDROP_C                    : 1;
	__u32      fgC121                      : 1;
	__u32      fgVYLNR                     : 1;
	__u32      fgVCLNR                     : 1;
	__u32      fgVYRPT                     : 1;
	__u32      fgVCPRT                     : 1;

	__u32      fgVLNR2                     : 1;
	__u32      fgVYTURN                    : 1;
	__u32      fgVCTURN                    : 1;
	__u32      fgWSAV_OFF                  : 1;
	__u32      fgRSAV_OFF                  : 1;
	__u32      fgLONGBUF                   : 1;
	__u32      fgONE_LINE                  : 1;
	__u32      fgKEEP_4N                   : 1;

	__u32      fgVPHASE16                  : 1;
	__u32      fgEVEN_FIR                  : 1;
	__u32      fgFIX_LINE0                 : 1;
	__u32      fgDIRECT_EQ0                : 1;
	__u32      fgFIR_LNR                   : 1;
	__u32      fgVUPSCALE                  : 1;
	__u32      fgHUPSCALE                  : 1;
	__u32      fgFORCE_VDE                 : 1;

	__u32      fgCCIR601                   : 1;
	__u32      fgEXTEND_VDE                : 1;
	__u32      fgVSYNC_POL                 : 1;
	__u32      fgHSYNC_POL                 : 1;
	__u32      fgVDE2_POL                  : 1;
	__u32      fgVDE2_EARLY                : 1;
	__u32      fgALL_FRM                   : 1;
	__u32      fgEXTEND_HDE                : 1;

	/* DWORD - 020 */
	__u32                                  : 32;

	/* DWORD - 024 */
	__u32                                  : 32;

	/* DWORD - 028 */
	__u32      u4YSSLTT                    : 8;
	__u32      u4YSSLBB                    : 8;
	__u32      u4YSSLBT                    : 8;
	__u32      u4YSSLTB                    : 8;

	/* DWORD - 02C */
	__u32      u4CSSLTT                    : 8;
	__u32      u4CSSLBB                    : 8;
	__u32      u4CSSLBT                    : 8;
	__u32      u4CSSLTB                    : 8;

	/* DWORD - 030 */
	__u32      fgY4TAP_ALL                 : 1;
	__u32      fgC4TAP_ALL                 : 1;
	__u32      fgPIC_FLD0                  : 1;
	__u32                                  : 1;
	__u32      fgHOLD_DIFF                 : 1;
	__u32      fgSLINEACT_SEL              : 1;
	__u32      fgFRC_TV_FLD                : 1;
	__u32      fgYC16BIT                   : 1;

	__u32      fgSELF_EN                   : 1;
	__u32      fgSVSCLON                   : 1;
	__u32      fgSHSCLON                   : 1;
	__u32      fgSHD_ON                    : 1;
	__u32      fgSHD_TYPE                  : 1;
	__u32      fgSPRGSOUT                  : 1;
	__u32                                  : 2;

	__u32      u4TESTMODE                  : 15;
	__u32      fgMIFOUT_HSYNC_SEL          : 1;  /* V up and H up please set 1*/

	/* DWORD - 034 */
	__u32                                  : 16;
	__u32      u4HWOPT                     : 8;
	__u32                                  : 8;

	/* DWORD - 038 */
	__u32      U4PXLSKIP                   : 11;
	__u32                                  : 21;

	/* DWORD - 03C */
	__u32      u4SCLRST                    : 8;
	__u32                                  : 24;

	/* DWORD - 040 */
	__u32      u4YV_COEF0M1                : 8;
	__u32      u4YV_COEF0_0                : 8;
	__u32      u4YV_COEF0_1                : 8;
	__u32                                  : 8;

	/* DWORD - 044 */
	__u32      u4YV_COEF1M1                : 8;
	__u32      u4YV_COEF1_0                : 8;
	__u32      u4YV_COEF1_1                : 8;
	__u32                                  : 8;


	/* DWORD - 048 */
	__u32      u4YV_COEF2M1                : 8;
	__u32      u4YV_COEF2_0                : 8;
	__u32      u4YV_COEF2_1                : 8;
	__u32                                  : 8;

	/* DWORD - 04C */
	__u32      u4YV_COEF3M1                : 8;
	__u32      u4YV_COEF3_0                : 8;
	__u32      u4YV_COEF3_1                : 8;
	__u32                                  : 8;

	/* DWORD - 050 */
	__u32      u4YV_COEF4M1                : 8;
	__u32      u4YV_COEF4_0                : 8;
	__u32      u4YV_COEF4_1                : 8;
	__u32                                  : 8;

	/* DWORD - 054 */
	__u32      u4YV_COEF5M1                : 8;
	__u32      u4YV_COEF5_0                : 8;
	__u32      u4YV_COEF5_1                : 8;
	__u32                                  : 8;

	/* DWORD - 058 */
	__u32      u4YV_COEF6M1                : 8;
	__u32      u4YV_COEF6_0                : 8;
	__u32      u4YV_COEF6_1                : 8;
	__u32                                  : 8;

	/* DWORD - 05C */
	__u32      u4YV_COEF7M1                : 8;
	__u32      u4YV_COEF7_0                : 8;
	__u32      u4YV_COEF7_1                : 8;
	__u32                                  : 8;

	/* DWORD - 060 */
	__u32      u4CV_COEF0M1                : 8;
	__u32      u4CV_COEF0_0                : 8;
	__u32      u4CV_COEF0_1                : 8;
	__u32                                  : 8;

	/* DWORD - 064 */
	__u32      u4CV_COEF1M1                : 8;
	__u32      u4CV_COEF1_0                : 8;
	__u32      u4CV_COEF1_1                : 8;
	__u32                                  : 8;

	/* DWORD - 068 */
	__u32      u4CV_COEF2M1                : 8;
	__u32      u4CV_COEF2_0                : 8;
	__u32      u4CV_COEF2_1                : 8;
	__u32                                  : 8;

	/* DWORD - 06C */
	__u32      u4CV_COEF3M1                : 8;
	__u32      u4CV_COEF3_0                : 8;
	__u32      u4CV_COEF3_1                : 8;
	__u32                                  : 8;

	/* DWORD - 070 */
	__u32      u4CV_COEF4M1                : 8;
	__u32      u4CV_COEF4_0                : 8;
	__u32      u4CV_COEF4_1                : 8;
	__u32                                  : 8;

	/* DWORD - 074 */
	__u32      u4CV_COEF5M1                : 8;
	__u32      u4CV_COEF5_0                : 8;
	__u32      u4CV_COEF5_1                : 8;
	__u32                                  : 8;

	/* DWORD - 078 */
	__u32      u4CV_COEF6M1                : 8;
	__u32      u4CV_COEF6_0                : 8;
	__u32      u4CV_COEF6_1                : 8;
	__u32                                  : 8;

	/* DWORD - 07C */
	__u32      u4CV_COEF7M1                : 8;
	__u32      u4CV_COEF7_0                : 8;
	__u32      u4CV_COEF7_1                : 8;
	__u32                                  : 8;

	/* DWORD - 080 */
	__u32      u4YV_COEF0_098              : 2;
	__u32      u4YV_COEF0_198              : 2;
	__u32      u4YV_COEF1_098              : 2;
	__u32      u4YV_COEF1_198              : 2;
	__u32      u4YV_COEF2_098              : 2;
	__u32      u4YV_COEF2_198              : 2;
	__u32      u4YV_COEF3_098              : 2;
	__u32      u4YV_COEF3_198              : 2;

	__u32      u4YV_COEF4_098              : 2;
	__u32      u4YV_COEF4_198              : 2;
	__u32      u4YV_COEF5_098              : 2;
	__u32      u4YV_COEF5_198              : 2;
	__u32      u4YV_COEF6_098              : 2;
	__u32      u4YV_COEF6_198              : 2;
	__u32      u4YV_COEF7_098              : 2;
	__u32      u4YV_COEF7_198              : 2;

	/* DWORD - 084 */
	__u32      u4CV_COEF0_098              : 2;
	__u32      u4CV_COEF0_198              : 2;
	__u32      u4CV_COEF1_098              : 2;
	__u32      u4CV_COEF1_198              : 2;
	__u32      u4CV_COEF2_098              : 2;
	__u32      u4CV_COEF2_198              : 2;
	__u32      u4CV_COEF3_098              : 2;
	__u32      u4CV_COEF3_198              : 2;

	__u32      u4CV_COEF4_098              : 2;
	__u32      u4CV_COEF4_198              : 2;
	__u32      u4CV_COEF5_098              : 2;
	__u32      u4CV_COEF5_198              : 2;
	__u32      u4CV_COEF6_098              : 2;
	__u32      u4CV_COEF6_198              : 2;
	__u32      u4CV_COEF7_098              : 2;
	__u32      u4CV_COEF7_198              : 2;

	/* DWORD - 088 */
	__u32                                  : 32;

	/* DWORD - 08C */
	__u32                                  : 10;
	__u32      fgMAHW_OPT                  : 1;
	__u32                                  : 5;

	__u32      u4SSLINETOP                 : 3;
	__u32                                  : 1;
	__u32      u4SSLINEBOT                 : 3;
	__u32                                  : 1;

	__u32      fgSYNCOUT_EN                : 1;
	__u32                                  : 3;
	__u32      fgDEL_Y                     : 1;
	__u32      fgDEL_c                     : 1;
	__u32                                  : 1;
	__u32      fgVUPLNR                    : 1;

	/* DWORD - 090 */
	__u32      u4YH_COEF0_0                : 8;
	__u32      u4YH_COEF0_1                : 8;
	__u32      u4YH_COEF0_2                : 8;
	__u32      u4YH_COEF0_3                : 8;

	/* DWORD - 094 */
	__u32      u4YH_COEF0_4                : 8;
	__u32      u4YH_COEF0_5                : 8;
	__u32      u4YH_COEF0_6                : 8;
	__u32      u4YH_COEF0_7                : 8;

	/* DWORD - 098 */
	__u32      u4YH_COEF1_0                : 8;
	__u32      u4YH_COEF1_1                : 8;
	__u32      u4YH_COEF1_2                : 8;
	__u32      u4YH_COEF1_3                : 8;

	/* DWORD - 09C */
	__u32      u4YH_COEF1_4                : 8;
	__u32      u4YH_COEF1_5                : 8;
	__u32      u4YH_COEF1_6                : 8;
	__u32      u4YH_COEF1_7                : 8;

	/* DWORD - 0A0 */
	__u32                                  : 32;

	/* DWORD - 0A4 */
	__u32                                  : 32;

	/* DWORD - 0A8 */
	__u32                                  : 32;

	/* DWORD - 0AC */
	__u32                                  : 32;

	/* DWORD - 0B0 */
	__u32                                  : 32;

	/* DWORD - 0B4 */
	__u32                                  : 32;

	/* DWORD - 0B8 */
	__u32                                  : 32;

	/* DWORD - 0BC */
	__u32                                  : 32;

	/* DWORD - 0C0 */
	__u32      u4YH_COEF2_0                : 8;
	__u32      u4YH_COEF2_1                : 8;
	__u32      u4YH_COEF2_2                : 8;
	__u32      u4YH_COEF2_3                : 8;

	/* DWORD - 0C4 */
	__u32      u4YH_COEF2_4                : 8;
	__u32      u4YH_COEF2_5                : 8;
	__u32      u4YH_COEF2_6                : 8;
	__u32      u4YH_COEF2_7                : 8;

	/* DWORD - 0C8 */
	__u32      u4YH_COEF3_0                : 8;
	__u32      u4YH_COEF3_1                : 8;
	__u32      u4YH_COEF3_2                : 8;
	__u32      u4YH_COEF3_3                : 8;

	/* DWORD - 0CC */
	__u32      u4YH_COEF3_4                : 8;
	__u32      u4YH_COEF3_5                : 8;
	__u32      u4YH_COEF3_6                : 8;
	__u32      u4YH_COEF3_7                : 8;

	/* DWORD - 0D0 */
	__u32      u4YH_COEF4_0                : 8;
	__u32      u4YH_COEF4_1                : 8;
	__u32      u4YH_COEF4_2                : 8;
	__u32      u4YH_COEF4_3                : 8;

	/* DWORD - 0D4 */
	__u32      u4YH_COEF4_4                : 8;
	__u32      u4YH_COEF4_5                : 8;
	__u32      u4YH_COEF4_6                : 8;
	__u32      u4YH_COEF4_7                : 8;

	/* DWORD - 0D8 */
	__u32      u4YH_COEF5_0                : 8;
	__u32      u4YH_COEF5_1                : 8;
	__u32      u4YH_COEF5_2                : 8;
	__u32      u4YH_COEF5_3                : 8;

	/* DWORD - 0DC */
	__u32      u4YH_COEF5_4                : 8;
	__u32      u4YH_COEF5_5                : 8;
	__u32      u4YH_COEF5_6                : 8;
	__u32      u4YH_COEF5_7                : 8;

	/* DWORD - 0E0 */
	__u32      u4YH_COEF6_0                : 8;
	__u32      u4YH_COEF6_1                : 8;
	__u32      u4YH_COEF6_2                : 8;
	__u32      u4YH_COEF6_3                : 8;

	/* DWORD - 0E4 */
	__u32      u4YH_COEF6_4                : 8;
	__u32      u4YH_COEF6_5                : 8;
	__u32      u4YH_COEF6_6                : 8;
	__u32      u4YH_COEF6_7                : 8;

	/* DWORD - 0E8 */
	__u32      u4YH_COEF7_0                : 8;
	__u32      u4YH_COEF7_1                : 8;
	__u32      u4YH_COEF7_2                : 8;
	__u32      u4YH_COEF7_3                : 8;

	/* DWORD - 0EC */
	__u32      u4YH_COEF7_4                : 8;
	__u32      u4YH_COEF7_5                : 8;
	__u32      u4YH_COEF7_6                : 8;
	__u32      u4YH_COEF7_7                : 8;

	/* DWORD - 0F0 */
	__u32      u4YH_COEF0_M1               : 8;
	__u32      u4YH_COEF0_8                : 8;
	__u32      u4YH_COEF1_M1               : 8;
	__u32      u4YH_COEF1_8                : 8;

	/* DWORD - 0F4 */
	__u32      u4YH_COEF2_M1               : 8;
	__u32      u4YH_COEF2_8                : 8;
	__u32      u4YH_COEF3_M1               : 8;
	__u32      u4YH_COEF3_8                : 8;

	/* DWORD - 0F8 */
	__u32      u4YH_COEF4_M1               : 8;
	__u32      u4YH_COEF4_8                : 8;
	__u32      u4YH_COEF5_M1               : 8;
	__u32      u4YH_COEF5_8                : 8;

	/* DWORD - 0FC */
	__u32      u4YH_COEF0_398              : 2;
	__u32      u4YH_COEF0_498              : 2;
	__u32      u4YH_COEF1_398              : 2;
	__u32      u4YH_COEF1_498              : 2;
	__u32      u4YH_COEF2_398              : 2;
	__u32      u4YH_COEF2_498              : 2;
	__u32      u4YH_COEF3_398              : 2;
	__u32      u4YH_COEF3_498              : 2;

	__u32      u4YH_COEF4_398              : 2;
	__u32      u4YH_COEF4_498              : 2;
	__u32      u4YH_COEF5_398              : 2;
	__u32      u4YH_COEF5_498              : 2;
	__u32      u4YH_COEF6_398              : 2;
	__u32      u4YH_COEF6_498              : 2;
	__u32      u4YH_COEF7_398              : 2;
	__u32      u4YH_COEF7_498              : 2;
} SCL_HAL_SCL_FIELD_T;

typedef union _SCL_HAL_SCL_UNION_T {
	__u32              au4Reg[SCL_HAL_SCL_REG_NUM];
	SCL_HAL_SCL_FIELD_T     rField;
} SCL_HAL_SCL_UNION_T;


typedef struct _SCL_HAL_FMT_FIELD_T {
	/* DWORD - 000 */
	__u32      u4CH_COEF0_0                : 8;
	__u32      u4CH_COEF0_1                : 8;
	__u32      u4CH_COEF0_2                : 8;
	__u32      u4CH_COEF0_3                : 8;

	/* DWORD - 004 */
	__u32      u4CH_COEF0_4                : 8;
	__u32      u4CH_COEF0_5                : 8;
	__u32      u4CH_COEF0_6                : 8;
	__u32      u4CH_COEF0_7                : 8;

	/* DWORD - 008 */
	__u32      u4CH_COEF1_0                : 8;
	__u32      u4CH_COEF1_1                : 8;
	__u32      u4CH_COEF1_2                : 8;
	__u32      u4CH_COEF1_3                : 8;

	/* DWORD - 00C */
	__u32      u4CH_COEF1_4                : 8;
	__u32      u4CH_COEF1_5                : 8;
	__u32      u4CH_COEF1_6                : 8;
	__u32      u4CH_COEF1_7                : 8;

	/* DWORD - 010 */
	__u32      u4CH_COEF2_0                : 8;
	__u32      u4CH_COEF2_1                : 8;
	__u32      u4CH_COEF2_2                : 8;
	__u32      u4CH_COEF2_3                : 8;

	/* DWORD - 014 */
	__u32      u4CH_COEF2_4                : 8;
	__u32      u4CH_COEF2_5                : 8;
	__u32      u4CH_COEF2_6                : 8;
	__u32      u4CH_COEF2_7                : 8;

	/* DWORD - 018 */
	__u32      u4CH_COEF3_0                : 8;
	__u32      u4CH_COEF3_1                : 8;
	__u32      u4CH_COEF3_2                : 8;
	__u32      u4CH_COEF3_3                : 8;

	/* DWORD - 01C */
	__u32      u4CH_COEF3_4                : 8;
	__u32      u4CH_COEF3_5                : 8;
	__u32      u4CH_COEF3_6                : 8;
	__u32      u4CH_COEF3_7                : 8;

	/* DWORD - 020 */
	__u32      u4CH_COEF4_0                : 8;
	__u32      u4CH_COEF4_1                : 8;
	__u32      u4CH_COEF4_2                : 8;
	__u32      u4CH_COEF4_3                : 8;

	/* DWORD - 024 */
	__u32      u4CH_COEF4_4                : 8;
	__u32      u4CH_COEF4_5                : 8;
	__u32      u4CH_COEF4_6                : 8;
	__u32      u4CH_COEF4_7                : 8;

	/* DWORD - 028 */
	__u32      u4CH_COEF5_0                : 8;
	__u32      u4CH_COEF5_1                : 8;
	__u32      u4CH_COEF5_2                : 8;
	__u32      u4CH_COEF5_3                : 8;

	/* DWORD - 02C */
	__u32      u4CH_COEF5_4                : 8;
	__u32      u4CH_COEF5_5                : 8;
	__u32      u4CH_COEF5_6                : 8;
	__u32      u4CH_COEF5_7                : 8;

	/* DWORD - 030 */
	__u32      u4CH_COEF6_0                : 8;
	__u32      u4CH_COEF6_1                : 8;
	__u32      u4CH_COEF6_2                : 8;
	__u32      u4CH_COEF6_3                : 8;

	/* DWORD - 034 */
	__u32      u4CH_COEF6_4                : 8;
	__u32      u4CH_COEF6_5                : 8;
	__u32      u4CH_COEF6_6                : 8;
	__u32      u4CH_COEF6_7                : 8;

	/* DWORD - 038 */
	__u32      u4CH_COEF7_0                : 8;
	__u32      u4CH_COEF7_1                : 8;
	__u32      u4CH_COEF7_2                : 8;
	__u32      u4CH_COEF7_3                : 8;

	/* DWORD - 03C */
	__u32      u4CH_COEF7_4                : 8;
	__u32      u4CH_COEF7_5                : 8;
	__u32      u4CH_COEF7_6                : 8;
	__u32      u4CH_COEF7_7                : 8;

	/* DWORD - 040 */
	__u32      u4CH_COEF6_M1               : 8;
	__u32      u4CH_COEF6_8                : 8;
	__u32      u4CH_COEF7_M1               : 8;
	__u32      u4CH_COEF7_8                : 8;

	/* DWORD - 044 */
	__u32                                  : 4;
	__u32      fgPAU2LEN                   : 1;
	__u32      fgPAU2LST                   : 1;
	__u32      fgPAUSSYN                   : 1;
	__u32                                  : 25;

	/* DWORD - 048 */
	__u32      u4HOR_END                   : 12;
	__u32                                  : 4;
	__u32      u4HOR_START                 : 12;
	__u32                                  : 4;

	/* DWORD - 04C */
	__u32      u4CH_COEF0_398              : 2;
	__u32      u4CH_COEF0_498              : 2;
	__u32      u4CH_COEF1_398              : 2;
	__u32      u4CH_COEF1_498              : 2;
	__u32      u4CH_COEF2_398              : 2;
	__u32      u4CH_COEF2_498              : 2;
	__u32      u4CH_COEF3_398              : 2;
	__u32      u4CH_COEF3_498              : 2;

	__u32      u4CH_COEF4_398              : 2;
	__u32      u4CH_COEF4_498              : 2;
	__u32      u4CH_COEF5_398              : 2;
	__u32      u4CH_COEF5_498              : 2;
	__u32      u4CH_COEF6_398              : 2;
	__u32      u4CH_COEF6_498              : 2;
	__u32      u4CH_COEF7_398              : 2;
	__u32      u4CH_COEF7_498              : 2;

	/* DWORD - 050 */
	__u32                                  : 32;

	/* DWORD - 054 */
	__u32                                  : 32;

	/* DWORD - 058 */
	__u32                                  : 32;

	/* DWORD - 05C */
	__u32                                  : 32;

	/* DWORD - 060 */
	__u32                                  : 32;

	/* DWORD - 064 */
	__u32                                  : 32;

	/* DWORD - 068 */
	__u32                                  : 32;

	/* DWORD - 06C */
	__u32                                  : 32;

	/* DWORD - 070 */
	__u32                                  : 32;

	/* DWORD - 074 */
	__u32                                  : 32;

	/* DWORD - 078 */
	__u32                                  : 32;

	/* DWORD - 07C */
	__u32     fgMASTER_EN                  : 1;
	__u32                                  : 2;
	__u32     fgFMT_HD_ON                  : 1;
	__u32     fgFMT_HD_TYPE                : 1;
	__u32     fgPROG_OUT                   : 1; /* must be 1*/
	__u32                                  : 10;

	__u32     fgHD_MODE                    : 1; /* 0: SD mode; 1: HD mode(bypass scaler)*/
	__u32                                  : 3;
	__u32     fgSEL_SHIFT                  : 1; /* must be 1*/
	__u32     fgDELAY_SEL                  : 1; /* 0: shift; 1: H/V total - shift*/
	__u32                                  : 10;

	/* DWORD - 080 */
	__u32      fgY_TURN                    : 1;
	__u32      fgC_TURN                    : 1;
	__u32      fgY8TAP_ALL                 : 1;
	__u32      fgC8TAP_ALL                 : 1;
	__u32      fgH_LNR2                    : 1;
	__u32                                  : 2;
	__u32      fgOPT7                      : 1;

	__u32      u4HSCALE2                   : 8;
	__u32                                  : 16;

	/* DWORD - 084 */
	__u32      fgWINON                     : 1;
	__u32      fgGETDIFF                   : 1;
	__u32      fgVDOIN_VSEL                : 1;
	__u32      fgSMLFMT_ACT                : 1;
	__u32      fgPRE_SYNC_EN               : 1;
	__u32                                  : 3;

	__u32      u4FISTPXLLEAD               : 7;
	__u32                                  : 1;

	__u32      u4SUBDENOM                  : 4;
	__u32      u4RESADD                    : 6;
	__u32      u4AVGADD                    : 6;

	/* DWORD - 088 */
	__u32      u4DIG_VEND                  : 8;
	__u32      u4DIG_VSTART                : 8;
	__u32      u4DIG_HWIDTH                : 8;

	__u32      fgDIG_VPOL                  : 1;
	__u32      fgDIG_HPOL                  : 1;
	__u32                                  : 2;
	__u32      fgDIG_OVS_OPT               : 1;
	__u32      fgDIG_OVE_OPT               : 1;
	__u32      fgDIG_EVS_OPT               : 1;
	__u32      fgDIG_EVE_OPT               : 1;

	/* DWORD - 08C */
	__u32      u4VTOTAL                    : 11;
	__u32                                  : 5;
	__u32      u4HTOTAL                    : 12;
	__u32      fgADJTOTAL                  : 1;
	__u32                                  : 3;

	/* DWORD - 090 */
	__u32      u4VSHIFT                    : 11;
	__u32                                  : 5;
	__u32      u4HSHIFT                    : 12;
	__u32      u4TESTMODE                  : 4;

	/* DWORD - 094 */
	__u32      u4HSYNWIDTH                 : 8;
	__u32      u4VSYNWIDTH                 : 5;
	__u32                                  : 3;

	__u32      fgDIG_VSYN                  : 1;
	__u32                                  : 13;
	__u32      fgTV_TYPE0                  : 1;
	__u32      fgTV_TYPE1                  : 1;

	/* DWORD - 098 */
	__u32      u4REG1098                   : 11;
	__u32                                  : 21;

	/* DWORD - 09C */
	__u32      u4PXLLENGTH                 : 12;
	__u32                                  : 20;

	/* DWORD - 0A0 */
	__u32      u4XACTEND                   : 12;
	__u32                                  : 4;
	__u32      u4XACTSTART                 : 12;
	__u32                                  : 4;

	/* DWORD - 0A4 */
	__u32      u4YOACTEND                  : 11;
	__u32                                  : 5;
	__u32      u4YOACTSTART                : 11;
	__u32                                  : 1;
	__u32      u4YOHIDESTART               : 4;

	/* DWORD - 0A8 */
	__u32      u4YEACTEND                  : 11;
	__u32                                  : 5;
	__u32      u4YEACTSTART                : 11;
	__u32                                  : 1;
	__u32      u4YEHIDESTART               : 4;

	/* DWORD - 0AC */
	__u32                                  : 1;
	__u32      fgFRM_SYNC_DIS              : 1;
	__u32                                  : 1;
	__u32      fgHSYNPOL                   : 1;
	__u32      fgVSYNPOL                   : 1;
	__u32                                  : 3;

	__u32      fgDE_720                    : 1;
	__u32      fgDE_720_LONG               : 1;
	__u32      fgRSTTRIG                   : 1;
	__u32      fgSHVSYN                    : 1;
	__u32                                  : 2;
	__u32      u4SYN_DEL                   : 2;

	__u32      fgDE_SEL                    : 1;
	__u32      fgDE_STA                    : 1;
	__u32      fgADE                       : 1;
	__u32      fgUV_SWAP                   : 1;
	__u32      fgBLK_RST                   : 1;
	__u32      fgRST_MD                    : 1;
	__u32                                  : 2;

	__u32      fgSYNC_ALL                  : 1;
	__u32      fgALL_BLK                   : 1;
	__u32      fgFIRST_PXLSEL              : 1;
	__u32      fgNO_PFLINE                 : 1;
	__u32      u4HWOPTION                  : 4;

	/* DWORD - 0B0 */
	__u32      fgHSCLON                    : 1;
	__u32      fgHYLNR                     : 1;
	__u32      fgHCLNR                     : 1;
	__u32      fgMULTIR                    : 1;
	__u32      fgHYRPT                     : 1;
	__u32      fgHCTWO                     : 1;
	__u32      fgHPHASE16                  : 1;
	__u32      fgHEVENFIR                  : 1;

	__u32      u4Y_ACC_STA                 : 4;
	__u32      u4C_ACC_STA                 : 4;

	__u32      u4HSCALE                    : 11;
	__u32                                  : 5;

	/* DWORD - 0B4 */
	__u32                                  : 4;
	__u32      u4BIY                       : 4;
	__u32                                  : 4;
	__u32      u4BICB                      : 4;
	__u32                                  : 4;

	__u32      u4BICR                      : 4;
	__u32      fgNEWPFDIS                  : 1;
	__u32      fgFORCEHIDE                 : 1;
	__u32                                  : 6;

	/* DWORD - 0B8 */
	__u32                                  : 4;
	__u32      u4BGY                       : 4;
	__u32                                  : 4;
	__u32      u4BGCB                      : 4;
	__u32                                  : 4;
	__u32      u4BGCR                      : 4;
	__u32                                  : 8;

	/* DWORD - 0BC */
	__u32      u4X_DIG_END                 : 12;
	__u32                                  : 4;
	__u32      u4X_DIG_START               : 12;
	__u32                                  : 4;

	/* DWORD - 0C0 */
	__u32      u4Y_DIG_OEND                : 11;
	__u32                                  : 5;
	__u32      u4Y_DIG_OSTART              : 11;
	__u32                                  : 5;

	/* DWORD - 0C4 */
	__u32      u4Y_DIG_EEND                : 11;
	__u32                                  : 5;
	__u32      u4Y_DIG_ESTART              : 11;
	__u32                                  : 5;

	/* DWORD - 0C8 */
	__u32      u4INCEND                    : 12;
	__u32      u4INCSTART                  : 12;
	__u32      u4INCSTEP                   : 8;

	/* DWORD - 0CC */
	__u32      u4DECEND                    : 12;
	__u32      u4DECSTART                  : 12;
	__u32      u4INITSCALE                 : 8;

	/* DWORD - 0D0 */
	__u32      u4XWINEND                   : 12;
	__u32                                  : 4;
	__u32      u4XWINSTART                 : 12;
	__u32                                  : 4;

	/* DWORD - 0D4 */
	__u32      u4YOWINEND                  : 11;
	__u32                                  : 5;
	__u32      u4YOWINSTART                : 11;
	__u32                                  : 5;

	/* DWORD - 0D8 */
	__u32      u4YEWINEND                  : 11;
	__u32                                  : 5;
	__u32      u4YEWINSTART                : 11;
	__u32                                  : 5;

	/* DWORD - 0DC */
	__u32      u4SLINE_ACT_E               : 11;
	__u32                                  : 5;
	__u32      u4SLINE_ACT_O               : 11;
	__u32                                  : 5;

	/* DWORD - 0E0 */
	__u32                                  : 32;

	/* DWORD - 0E4 */
	__u32                                  : 32;

	/* DWORD - 0E8 */
	__u32      u4LAST_HTOTAL               : 12; /* must be h total in master mode*/
	__u32                                  : 4;
	__u32      u4HTOTAL_IN                 : 12;
	__u32                                  : 4;

	/* DWORD - 0EC */
	__u32      u4TV_PIXEL_MON              : 12;
	__u32                                  : 4;
	__u32      u4TV_LINE_MON               : 11;
	__u32                                  : 5;

	/* DWORD - 0F0 */
	__u32                                  : 32;

	/* DWORD - 0F4 */
	__u32                                  : 32;

	/* DWORD - 0F8 */
	__u32                                  : 32;

	/* DWORD - 0FC */
	__u32                                  : 32;
} SCL_HAL_FMT_FIELD_T;

typedef union _HAL_SCL_FMT_UNION_T {
	__u32              au4Reg[SCL_HAL_FMT_REG_NUM];
	SCL_HAL_FMT_FIELD_T     rField;
} SCL_HAL_FMT_UNION_T;

#define GET_PMX_MIX_HWPTR(reg, mode) \
	do { \
		reg = (PMX_HAL_MIX_UNION_T *)mix_reg; \
		mode = _rPmxMixRegMode; \
	} while (0)
#endif  /* _SCL_HW_H_*/




