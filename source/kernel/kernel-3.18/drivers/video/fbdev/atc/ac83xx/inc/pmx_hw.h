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
       
#ifndef _PMX_HW_H_
#define _PMX_HW_H_

 #include "x_hal_io.h"
#include "x_hal_ic.h"
#include "drv_config.h"

#include "chip_ver.h"


#ifdef __linux__

#ifndef IO_BASE_VA
#define IO_BASE_VA     0xFD000000
#endif

#else
#define IO_BASE_VA                            0xA0000000
#endif

#define PMX_HAL_DISP_MAIN_REG             (IO_BASE_VA + 0x42000)
#define PMX_HAL_DISP_AUX_REG              (IO_BASE_VA + 0x43000)
#define PMX_HAL_MIX_REG                   (IO_BASE_VA + 0x1F000)

#define PMX_HAL_DISP_MAIN_REG_NUM         (0x100/4)
#define PMX_HAL_DISP_AUX_REG_NUM          (0x100/4)
#define PMX_HAL_MIX_REG_NUM               (0x84/4)

#define PMX_HAL_REG_MODE_NULL             0
#define PMX_HAL_REG_MODE_READ             (1 << 0)
#define PMX_HAL_REG_MODE_WRITE            (1 << 1)
/* Time sync between VDP & PMX API, Since VDP_MSG_Q_T will delay 1 VSYNC*/
#define PMX_HAL_REG_MODE_DELAY_WRITE      (1 << 3)


typedef struct _PMX_HAL_DISP_MAIN_FIELD_T {
	/* DWORD - 000 */
	__u32                                        : 32;

	/* DWORD - 004 */
	__u32                                        : 32;

	/* DWORD - 008 */
	__u32                                        : 32;

	/* DWORD - 00C */
	__u32                                        : 32;

	/* DWORD - 010 */
	__u32                                        : 32;

	/* DWORD - 014 */
	__u32                                        : 32;

	/* DWORD - 018 */
	__u32                                        : 32;

	/* DWORD - 01C */
	__u32                                        : 32;

	/* DWORD - 020 */
	__u32                                        : 32;

	/* DWORD - 024 */
	__u32                                        : 32;

	/* DWORD - 028 */
	__u32                                        : 32;

	/* DWORD - 02C */
	__u32                                        : 32;

	/* DWORD - 030 */
	__u32                                        : 32;

	/* DWORD - 034 */
	__u32                                        : 32;

	/* DWORD - 038 */
	__u32                                        : 32;

	/* DWORD - 03C */
	__u32                                        : 32;

	/* DWORD - 040 */
	__u32                                        : 32;

	/* DWORD - 044 */
	__u32                                        : 32;

	/* DWORD - 048 */
	__u32                                        : 32;

	/* DWORD - 04C */
	__u32                                        : 32;

	/* DWORD - 050 */
	__u32                                        : 32;

	/* DWORD - 054 */
	__u32                                        : 32;

	/* DWORD - 058 */
	__u32                                        : 32;

	/* DWORD - 05C */
	__u32                                        : 32;

	/* DWORD - 060 */
	__u32                                        : 32;

	/* DWORD - 064 */
	__u32                                        : 32;

	/* DWORD - 068 */
	__u32                                        : 32;

	/* DWORD - 06C */
	__u32                                        : 32;

	/* DWORD - 070 */
	__u32     fgShadowEn                         : 1;
	__u32                                        : 15;
	__u32     fgShadowVsyncSel                   : 1;
	__u32                                        : 14;
	__u32     fgShadowTriger                     : 1;

	/* DWORD - 074 */
	__u32     fgMasterSel                        : 1; /* scl master 1 for 8317; scl slave 0 for 3360*/
	__u32     fgSalveEn                          : 1; /* scl master 1; scl slave 0*/
	__u32                                        : 30;

	/* DWORD - 078 */
	__u32      fgEn_601_709                      : 1;
	__u32      fgC_709_2_601                     : 1;
	__u32                                        : 2;
	__u32      fgEn235_255                       : 1;
	__u32      fgC_235_2_255                     : 1;
	__u32                                        : 26;

	/* DWORD - 07C */
	__u32      u4TvPixel                         : 12;
	__u32                                        : 4;
	__u32      u4TvLine                          : 11;
	__u32                                        : 5;

	/* DWORD - 080 */
	__u32      u4OSD_EST                         : 7;
	__u32                                        : 1;
	__u32      u4OSD_OST                         : 7;
	__u32                                        : 1;

	__u32      u4SPMTE_SF                        : 11;
	__u32      fgF_POL                           : 1;
	__u32      fgV_POL                           : 1;
	__u32      fgSMPTE_EN                        : 1;
	__u32      fgSSF                             : 1;
	__u32                                        : 1;

	/* DWORD - 084 */
	__u32                                        : 32;

	/* DWORD - 088 */
	__u32      u4SPV_VEND                        : 11;
	__u32                                        : 5;
	__u32      u4SPV_VST                         : 11;
	__u32                                        : 5;

	/* DWORD - 08C */
	__u32      u4SPV_HEND                        : 12;
	__u32                                        : 4;
	__u32      u4SPV_HST                         : 12;
	__u32                                        : 4;

	/* DWORD - 090 */
	__u32      u4SP_H_END                        : 12;
	__u32                                        : 4;
	__u32      u4SP_H_START                      : 12;
	__u32                                        : 4;

	/* DWORD - 094 */
	__u32      u4HSYNWIDTH                       : 8;
	__u32      u4VSYNWIDTH                       : 5;
	__u32      fgHD_TP                           : 1;
	__u32      fgHD_ON                           : 1;
	__u32      fgPRGS                            : 1;

	__u32      u4SP_ESTART                       : 7;
	__u32      u4SP_OSTART6                      : 1;
	__u32      u4SP_OSTART                       : 6;
	__u32      u4TVMODE                          : 2;

	/* DWORD - 098 */
	__u32      u4HSYNOFF                         : 11;
	__u32                                        : 5;

	__u32      u4PF_ADV                          : 4;
	__u32                                        : 4;
	__u32      u4SMPTE_BGN                       : 7;
	__u32                                        : 1;

	/* DWORD - 09C */
	__u32      u4PXLLEN                          : 12;
	__u32                                        : 18;
	__u32      fgFrmLockEn                       : 1;  /* 0 FMT self free run; not support in 8317*/
	__u32      fgRstMode                         : 1;  /* 0 reset by 0xac[10]; 1 reset by vsync*/

	/* DWORD - 0A0 */
	__u32      u4HACTEND                         : 12;
	__u32                                        : 4;
	__u32      u4HACTBGN                         : 12;
	__u32                                        : 4;

	/* DWORD - 0A4 */
	__u32      u4VOACTEND                        : 11;
	__u32                                        : 5;
	__u32      u4VOACTBGN                        : 11;
	__u32                                        : 1;
	__u32      u4HIDE_OST                        : 4;

	/* DWORD - 0A8 */
	__u32      u4VEACTEND                        : 11;
	__u32                                        : 5;
	__u32      u4VEACTBGN                        : 11;
	__u32                                        : 1;
	__u32      u4HIDE_EST                        : 4;

	/* DWORD - 0AC */
	__u32      fgVDO_EN                          : 1;
	__u32      fgFMTM                            : 1;
	__u32      fgFIELD                           : 1;
	__u32      fgHPOR                            : 1;
	__u32      fgVPOR                            : 1;
	__u32      fgFINV                            : 1;
	__u32      fgSYNEN                           : 1;
	__u32      fgIG_1V                           : 1;

	__u32      u4PXLSEL                          : 2;
	__u32      fgFTRST                           : 1;
	__u32                                        : 3;
	__u32      u4SYN_DEL                         : 2;

	__u32      fgCLK_EDGE0                       : 1;
	__u32      fgCLK_EDGE1                       : 1;
	__u32      fgUVSW2                           : 1;
	__u32      fgUVSW                            : 1;
	__u32      fgCDELAY                          : 1;
	__u32      fgSP_DS                           : 1;
	__u32      fgYLMT_B                          : 1;
	__u32      fgYLMT_T                          : 1;

	__u32      fgMIX_EN                          : 1;
	__u32      fgBLACK                           : 1;
	__u32                                        : 1;
	__u32      fgPFOFF                           : 1;
	__u32      u4HW_OPT                          : 4;

	/* DWORD - 0B0 */
	__u32      fgHSON                            : 1;
	__u32      fgHSLR                            : 1;
	__u32      fgLPF_ON                          : 1;
	__u32      fgLPF_SL                          : 1;
	__u32      u4HD_LPF                          : 2;
	__u32      fgED_YUV_ST                       : 1;
	__u32      fgO_DIS                           : 1;

	__u32      u4YACC_ST                         : 4;
	__u32      u4CACC_ST                         : 4;
	__u32      u4HSFACTOR                        : 10;
	__u32                                        : 6;

	/* DWORD - 0B4 */
	__u32                                        : 4;
	__u32      u4BIY                             : 4;
	__u32                                        : 4;
	__u32      u4BICB                            : 4;
	__u32                                        : 4;
	__u32      u4BICR                            : 4;

	__u32      fgPF2OFF                          : 1;
	__u32      fgHIDE_L                          : 1;
	__u32                                        : 6;

	/* DWORD - 0B8 */
	__u32                                        : 4;
	__u32      u4BGY                             : 4;
	__u32                                        : 4;
	__u32      u4BGCB                            : 4;

	__u32                                        : 4;
	__u32      u4BGCR                            : 4;
	__u32                                        : 8;

	/* DWORD - 0BC */
	__u32      u4EDGE_RATIO                      : 5;
	__u32      fgKNEE                            : 1;
	__u32      fgZCORE                           : 1;
	__u32      fgDNR                             : 1;

	__u32      u4CORE                            : 4;
	__u32      u4DOUT_CTL                        : 3;
	__u32                                        : 15;
	__u32      fgVSYNC_SEL                       : 1;
	__u32      fgHSYNC_SEL                       : 1;

	/* DWORD - 0C0 */
	__u32                                        : 32;

	/* DWORD - 0C4 */
	__u32      u4Y_LMT_TOP                       : 8;
	__u32      u4Y_LMT_BOT                       : 8;

	__u32                                        : 1;
	__u32      fgOSD_DwnS                        : 1;
	__u32                                        : 1;
	__u32      fgCCIR_M                          : 1;
	__u32                                        : 4;
	__u32      u4FIRST_PXL_LEAD                  : 8;

	/* DWORD - 0C8 */
	__u32                                        : 32;

	/* DWORD - 0CC */
	__u32      fgP2I_FLD                         : 1;
	__u32      fgPFDAT                           : 1;
	__u32      fgPFDINV                          : 1;
	__u32      fgSFDAT                           : 1;
	__u32                                        : 2;
	__u32      fgMIXOSD                          : 1;
	__u32                                        : 2;
	__u32      fgDCK90p                          : 1;
	__u32      fgTVEDGO                          : 1;
	__u32      fgESAVI                           : 1;
	__u32      u4CCIR_MOD                        : 4;

	__u32      u4DCK_DELAY                       : 3;
	__u32      fgSYN_LS                          : 1;
	__u32      fgSYN_ADJ                         : 1;
	__u32      fgESAV4                           : 1;
	__u32                                        : 2;

	__u32      u4HSYN_SEL                        : 3;
	__u32      u4VSYN_SEL                        : 3;
	__u32      fgHV_EN                           : 1;
	__u32      fgDV_PP                           : 1;

	/* DWORD - 0D0 */
	__u32      u4CCIR_HEND                       : 12;
	__u32                                        : 4;
	__u32      u4CCIR_HBGN                       : 12;
	__u32                                        : 3;
	__u32      fgCCIR                            : 1;

	/* DWORD - 0D4 */
	__u32      u4V_TOTAL                         : 11;
	__u32                                        : 5;
	__u32      u4H_TOTAL                         : 12;
	__u32      fgADJ_T                           : 1;
	__u32                                        : 3;

	/* DWORD - 0D8 */
	__u32      u4INC_END                         : 12;
	__u32      u4INC_START                       : 12;
	__u32      u4INC_STEP                        : 8;

	/* DWORD - 0DC */
	__u32      u4DEC_END                         : 12;
	__u32      u4DEC_START                       : 12;
	__u32      u4INIT_SCALE                      : 8;

	/* DWORD - 0E0 */
	__u32      u4CCIR_VOEND                      : 11;
	__u32                                        : 5;
	__u32      u4CCIR_VOBGN                      : 11;
	__u32                                        : 4;
	__u32      fgPFLD                            : 1;

	/* DWORD - 0E4 */
	__u32                                        : 1;
	__u32      fgDIRECT                          : 1;
	__u32                                        : 2;
	__u32      fgADJS_F                          : 1;
	__u32                                        : 7;
	__u32      fgEXT_YUV_SEL                     : 1;
	__u32      fgMIXER_OFF                       : 1;
	__u32      fgRST_PHASE                       : 1;
	__u32                                        : 5;
	__u32      fgDGOY_LMT_TOP                    : 1;
	__u32      fgDGOY_LMT_BOT                    : 1;
	__u32      fgDGOC_656_C                      : 1;
	__u32      fgDGOC_LMT_CBCR                   : 1;

	__u32                                        : 4;
	__u32      fgHD_1080P_HALF                   : 1;
	__u32      fgMIX_709                         : 1;
	__u32      fgSCL_709                         : 1;
	__u32                                        : 1;

	/* DWORD - 0E8 */
	__u32      u4HSYN_DELAY                      : 12;
	__u32      u4VSYN_DELAY                      : 4;
	__u32      u4ADJ_H_L                         : 10;
	__u32      u4ADJ_V_L                         : 6;

	/* DWORD - 0EC */
	__u32      u4WIN_HEND                        : 12;
	__u32                                        : 4;
	__u32      u4WIN_HBGN                        : 12;
	__u32                                        : 3;
	__u32      fgWIN_ON                          : 1;

	/* DWORD - 0F0 */
	__u32      u4CCIR_VEEND                      : 11;
	__u32                                        : 5;
	__u32      u4CCIR_VEBGN                      : 11;
	__u32                                        : 5;

	/* DWORD - 0F4 */
	__u32      fgCB_ON                           : 1;
	__u32      fgCB_TP                           : 1;
	__u32                                        : 2;
	__u32      u4Y_DLY                           : 2;
	__u32      fgSP_DLY                          : 1;

	__u32                                        : 1;
	__u32      fgOSD_DLY                         : 1;
	__u32      fgYUV709_EN                       : 1;
	__u32      fgC_SIGN                          : 1;
	__u32      u4DGO_SEL                         : 2;
	__u32      fgDGO_LPF_EN                      : 1;
	__u32      fgDGO_LPF_RND                     : 1;

	__u32      u4DGO_LPF_TYPE                    : 2;
	__u32      fgMVDO_709                        : 1;
	__u32                                        : 3;
	__u32      fgMIX_SP_SIGN                     : 1;
	__u32                                        : 9;

	/* DWORD - 0F8 */
	__u32      u4WIN_VOEND                       : 11;
	__u32                                        : 5;
	__u32      u4WIN_VOBGN                       : 11;
	__u32                                        : 5;

	/* DWORD - 0FC */
	__u32      u4WIN_VEEND                       : 11;
	__u32                                        : 5;
	__u32      u4WIN_VEBGN                       : 11;
	__u32                                        : 5;
} PMX_HAL_DISP_MAIN_FIELD_T;

typedef union _PMX_HAL_DISP_MAIN_UNION_T {
	__u32              au4Reg[PMX_HAL_DISP_MAIN_REG_NUM];
	PMX_HAL_DISP_MAIN_FIELD_T   rField;
} PMX_HAL_DISP_MAIN_UNION_T;

typedef struct _PMX_HAL_DISP_AUX_FIELD_T {
	/* DWORD - 000 */
	__u32                                        : 32;

	/* DWORD - 004 */
	__u32                                        : 32;

	/* DWORD - 008 */
	__u32                                        : 32;

	/* DWORD - 00C */
	__u32                                        : 32;

	/* DWORD - 010 */
	__u32                                        : 32;

	/* DWORD - 014 */
	__u32                                        : 32;

	/* DWORD - 018 */
	__u32                                        : 32;

	/* DWORD - 01C */
	__u32                                        : 32;

	/* DWORD - 020 */
	__u32                                        : 32;

	/* DWORD - 024 */
	__u32                                        : 32;

	/* DWORD - 028 */
	__u32                                        : 32;

	/* DWORD - 02C */
	__u32                                        : 32;

	/* DWORD - 030 */
	__u32                                        : 32;

	/* DWORD - 034 */
	__u32                                        : 32;

	/* DWORD - 038 */
	__u32                                        : 32;

	/* DWORD - 03C */
	__u32                                        : 32;

	/* DWORD - 040 */
	__u32                                        : 32;

	/* DWORD - 044 */
	__u32                                        : 32;

	/* DWORD - 048 */
	__u32                                        : 32;

	/* DWORD - 04C */
	__u32                                        : 32;

	/* DWORD - 050 */
	__u32                                        : 32;

	/* DWORD - 054 */
	__u32                                        : 32;

	/* DWORD - 058 */
	__u32                                        : 32;

	/* DWORD - 05C */
	__u32                                        : 32;

	/* DWORD - 060 */
	__u32                                        : 32;

	/* DWORD - 064 */
	__u32                                        : 32;

	/* DWORD - 068 */
	__u32                                        : 32;

	/* DWORD - 06C */
	__u32                                        : 32;

	/* DWORD - 070 */
	__u32     fgShadowEn                         : 1;
	__u32                                        : 15;
	__u32     fgShadowVsyncSel                   : 1;
	__u32                                        : 14;
	__u32     fgShadowTriger                     : 1;

	/* DWORD - 074 */
	__u32     fgMasterSel                        : 1; /* scl master 1 for 8317; scl slave 0 for 3360*/
	__u32     fgSalveEn                          : 1; /* scl master 1; scl slave 0*/
	__u32                                        : 30;

	/* DWORD - 078 */
	__u32      fgEn_601_709                      : 1;
	__u32      fgC_709_2_601                     : 1;
	__u32                                        : 2;
	__u32      fgEn235_255                       : 1;
	__u32      fgC_235_2_255                     : 1;
	__u32                                        : 26;

	/* DWORD - 07C */
	__u32      u4TvPixel                         : 12;
	__u32                                        : 4;
	__u32      u4TvLine                          : 11;
	__u32                                        : 5;

	/* DWORD - 080 */
	__u32      u4OSD_EST                         : 7;
	__u32                                        : 1;
	__u32      u4OSD_OST                         : 7;
	__u32                                        : 1;

	__u32      u4SPMTE_SF                        : 11;
	__u32      fgF_POL                           : 1;
	__u32      fgV_POL                           : 1;
	__u32      fgSMPTE_EN                        : 1;
	__u32      fgSSF                             : 1;
	__u32                                        : 1;

	/* DWORD - 084 */
	__u32                                        : 32;

	/* DWORD - 088 */
	__u32      u4SPV_VEND                        : 11;
	__u32                                        : 5;
	__u32      u4SPV_VST                         : 11;
	__u32                                        : 5;

	/* DWORD - 08C */
	__u32      u4SPV_HEND                        : 12;
	__u32                                        : 4;
	__u32      u4SPV_HST                         : 12;
	__u32                                        : 4;

	/* DWORD - 090 */
	__u32      u4SP_H_END                        : 12;
	__u32                                        : 4;
	__u32      u4SP_H_START                      : 12;
	__u32                                        : 4;

	/* DWORD - 094 */
	__u32      u4HSYNWIDTH                       : 8;
	__u32      u4VSYNWIDTH                       : 5;
	__u32      fgHD_TP                           : 1;
	__u32      fgHD_ON                           : 1;
	__u32      fgPRGS                            : 1;

	__u32      u4SP_ESTART                       : 7;
	__u32      u4SP_OSTART6                      : 1;
	__u32      u4SP_OSTART                       : 6;
	__u32      u4TVMODE                          : 2;

	/* DWORD - 098 */
	__u32      u4HSYNOFF                         : 11;
	__u32                                        : 5;

	__u32      u4PF_ADV                          : 4;
	__u32                                        : 4;
	__u32      u4SMPTE_BGN                       : 7;
	__u32                                        : 1;

	/* DWORD - 09C */
	__u32      u4PXLLEN                          : 12;
	__u32                                        : 18;
	__u32      fgFrmLockEn                       : 1;  /* 0 FMT self free run; not support in 8317*/
	__u32      fgRstMode                         : 1;  /* 0 reset by 0xac[10]; 1 reset by vsync*/

	/* DWORD - 0A0 */
	__u32      u4HACTEND                         : 12;
	__u32                                        : 4;
	__u32      u4HACTBGN                         : 12;
	__u32                                        : 4;

	/* DWORD - 0A4 */
	__u32      u4VOACTEND                        : 11;
	__u32                                        : 5;
	__u32      u4VOACTBGN                        : 11;
	__u32                                        : 1;
	__u32      u4HIDE_OST                        : 4;

	/* DWORD - 0A8 */
	__u32      u4VEACTEND                        : 11;
	__u32                                        : 5;
	__u32      u4VEACTBGN                        : 11;
	__u32                                        : 1;
	__u32      u4HIDE_EST                        : 4;

	/* DWORD - 0AC */
	__u32      fgVDO_EN                          : 1;
	__u32      fgFMTM                            : 1;
	__u32      fgFIELD                           : 1;
	__u32      fgHPOR                            : 1;
	__u32      fgVPOR                            : 1;
	__u32      fgFINV                            : 1;
	__u32      fgSYNEN                           : 1;
	__u32      fgIG_1V                           : 1;

	__u32      u4PXLSEL                          : 2;
	__u32      fgFTRST                           : 1;
	__u32                                        : 3;
	__u32      u4SYN_DEL                         : 2;

	__u32      fgCLK_EDGE0                       : 1;
	__u32      fgCLK_EDGE1                       : 1;
	__u32      fgUVSW2                           : 1;
	__u32      fgUVSW                            : 1;
	__u32      fgCDELAY                          : 1;
	__u32      fgSP_DS                           : 1;
	__u32      fgYLMT_B                          : 1;
	__u32      fgYLMT_T                          : 1;

	__u32      fgMIX_EN                          : 1;
	__u32      fgBLACK                           : 1;
	__u32                                        : 1;
	__u32      fgPFOFF                           : 1;
	__u32      u4HW_OPT                          : 4;

	/* DWORD - 0B0 */
	__u32      fgHSON                            : 1;
	__u32      fgHSLR                            : 1;
	__u32      fgLPF_ON                          : 1;
	__u32      fgLPF_SL                          : 1;
	__u32      u4HD_LPF                          : 2;
	__u32      fgED_YUV_ST                       : 1;
	__u32      fgO_DIS                           : 1;

	__u32      u4YACC_ST                         : 4;
	__u32      u4CACC_ST                         : 4;
	__u32      u4HSFACTOR                        : 10;
	__u32                                        : 6;

	/* DWORD - 0B4 */
	__u32                                        : 4;
	__u32      u4BIY                             : 4;
	__u32                                        : 4;
	__u32      u4BICB                            : 4;
	__u32                                        : 4;
	__u32      u4BICR                            : 4;

	__u32      fgPF2OFF                          : 1;
	__u32      fgHIDE_L                          : 1;
	__u32                                        : 6;

	/* DWORD - 0B8 */
	__u32                                        : 4;
	__u32      u4BGY                             : 4;
	__u32                                        : 4;
	__u32      u4BGCB                            : 4;

	__u32                                        : 4;
	__u32      u4BGCR                            : 4;
	__u32                                        : 8;

	/* DWORD - 0BC */
	__u32      u4EDGE_RATIO                      : 5;
	__u32      fgKNEE                            : 1;
	__u32      fgZCORE                           : 1;
	__u32      fgDNR                             : 1;

	__u32      u4CORE                            : 4;
	__u32      u4DOUT_CTL                        : 3;
	__u32                                        : 15;
	__u32      fgVSYNC_SEL                       : 1;
	__u32      fgHSYNC_SEL                       : 1;

	/* DWORD - 0C0 */
	__u32                                        : 32;

	/* DWORD - 0C4 */
	__u32      u4Y_LMT_TOP                       : 8;
	__u32      u4Y_LMT_BOT                       : 8;

	__u32                                        : 1;
	__u32      fgOSD_DwnS                        : 1;
	__u32                                        : 1;
	__u32      fgCCIR_M                          : 1;
	__u32                                        : 4;
	__u32      u4FIRST_PXL_LEAD                  : 8;

	/* DWORD - 0C8 */
	__u32                                        : 32;

	/* DWORD - 0CC */
	__u32      fgP2I_FLD                         : 1;
	__u32      fgPFDAT                           : 1;
	__u32      fgPFDINV                          : 1;
	__u32      fgSFDAT                           : 1;
	__u32                                        : 2;
	__u32      fgMIXOSD                          : 1;
	__u32                                        : 2;
	__u32      fgDCK90p                          : 1;
	__u32      fgTVEDGO                          : 1;
	__u32      fgESAVI                           : 1;
	__u32      u4CCIR_MOD                        : 4;

	__u32      u4DCK_DELAY                       : 3;
	__u32      fgSYN_LS                          : 1;
	__u32      fgSYN_ADJ                         : 1;
	__u32      fgESAV4                           : 1;
	__u32                                        : 2;

	__u32      u4HSYN_SEL                        : 3;
	__u32      u4VSYN_SEL                        : 3;
	__u32      fgHV_EN                           : 1;
	__u32      fgDV_PP                           : 1;

	/* DWORD - 0D0 */
	__u32      u4CCIR_HEND                       : 12;
	__u32                                        : 4;
	__u32      u4CCIR_HBGN                       : 12;
	__u32                                        : 3;
	__u32      fgCCIR                            : 1;

	/* DWORD - 0D4 */
	__u32      u4V_TOTAL                         : 11;
	__u32                                        : 5;
	__u32      u4H_TOTAL                         : 12;
	__u32      fgADJ_T                           : 1;
	__u32                                        : 3;

	/* DWORD - 0D8 */
	__u32      u4INC_END                         : 12;
	__u32      u4INC_START                       : 12;
	__u32      u4INC_STEP                        : 8;

	/* DWORD - 0DC */
	__u32      u4DEC_END                         : 12;
	__u32      u4DEC_START                       : 12;
	__u32      u4INIT_SCALE                      : 8;

	/* DWORD - 0E0 */
	__u32      u4CCIR_VOEND                      : 11;
	__u32                                        : 5;
	__u32      u4CCIR_VOBGN                      : 11;
	__u32                                        : 4;
	__u32      fgPFLD                            : 1;

	/* DWORD - 0E4 */
	__u32                                        : 1;
	__u32      fgDIRECT                          : 1;
	__u32                                        : 2;
	__u32      fgADJS_F                          : 1;
	__u32                                        : 7;
	__u32      fgEXT_YUV_SEL                     : 1;
	__u32      fgMIXER_OFF                       : 1;
	__u32      fgRST_PHASE                       : 1;
	__u32                                        : 5;
	__u32      fgDGOY_LMT_TOP                    : 1;
	__u32      fgDGOY_LMT_BOT                    : 1;
	__u32      fgDGOC_656_C                      : 1;
	__u32      fgDGOC_LMT_CBCR                   : 1;

	__u32                                        : 4;
	__u32      fgHD_1080P_HALF                   : 1;
	__u32      fgMIX_709                         : 1;
	__u32      fgSCL_709                         : 1;
	__u32                                        : 1;

	/* DWORD - 0E8 */
	__u32      u4HSYN_DELAY                      : 12;
	__u32      u4VSYN_DELAY                      : 4;
	__u32      u4ADJ_H_L                         : 10;
	__u32      u4ADJ_V_L                         : 6;

	/* DWORD - 0EC */
	__u32      u4WIN_HEND                        : 12;
	__u32                                        : 4;
	__u32      u4WIN_HBGN                        : 12;
	__u32                                        : 3;
	__u32      fgWIN_ON                          : 1;

	/* DWORD - 0F0 */
	__u32      u4CCIR_VEEND                      : 11;
	__u32                                        : 5;
	__u32      u4CCIR_VEBGN                      : 11;
	__u32                                        : 5;

	/* DWORD - 0F4 */
	__u32      fgCB_ON                           : 1;
	__u32      fgCB_TP                           : 1;
	__u32                                        : 2;
	__u32      u4Y_DLY                           : 2;
	__u32      fgSP_DLY                          : 1;

	__u32                                        : 1;
	__u32      fgOSD_DLY                         : 1;
	__u32      fgYUV709_EN                       : 1;
	__u32      fgC_SIGN                          : 1;
	__u32      u4DGO_SEL                         : 2;
	__u32      fgDGO_LPF_EN                      : 1;
	__u32      fgDGO_LPF_RND                     : 1;

	__u32      u4DGO_LPF_TYPE                    : 2;
	__u32      fgMVDO_709                        : 1;
	__u32                                        : 3;
	__u32      fgMIX_SP_SIGN                     : 1;
	__u32                                        : 9;

	/* DWORD - 0F8 */
	__u32      u4WIN_VOEND                       : 11;
	__u32                                        : 5;
	__u32      u4WIN_VOBGN                       : 11;
	__u32                                        : 5;

	/* DWORD - 0FC */
	__u32      u4WIN_VEEND                       : 11;
	__u32                                        : 5;
	__u32      u4WIN_VEBGN                       : 11;
	__u32                                        : 5;
} PMX_HAL_DISP_AUX_FIELD_T;

typedef union _PMX_HAL_DISP_AUX_UNION_T {
	__u32              au4Reg[PMX_HAL_DISP_AUX_REG_NUM];
	PMX_HAL_DISP_AUX_FIELD_T    rField;
} PMX_HAL_DISP_AUX_UNION_T;

typedef struct _PMX_HAL_MIX_FIELD_T {
	/* DWORD - 000 */
	__u32        fgVIDEO_SRC_SEL                 : 1; /* 0 scaler and 1 front fmt*/
	__u32        fgVIDEO_MIX_EN                  : 1;
	__u32        fgOSD1_MIX_EN                   : 1;
	__u32        fgOSD2_MIX_EN                   : 1;
	__u32        fgOSD3_MIX_EN                   : 1;
	__u32        fgOSD4_MIX_EN                   : 1;
	__u32                                        : 2;

	__u32        u4MIX_LAYER1_SEL                : 3;
	__u32        fgDST_SEL_1                     : 1;
	__u32        u4MIX_LAYER2_SEL                : 3;
	__u32        fgDST_SEL_2                     : 1;

	__u32        u4MIX_LAYER3_SEL                : 3;
	__u32        fgDST_SEL_3                     : 1;
	__u32                                        : 4;

	__u32        u4MIX_LAYER4_SEL                : 3;
	__u32        fgDST_SEL_4                     : 1;
	__u32                                        : 2;
	__u32        fgVDO_TIMING_SEL                : 1; /* 0 scaler and 1 front fmt*/
	__u32        fgVIDEO_A_ADJ                   : 1;

	/* DWORD - 004 */
	__u32        u4VIDEO_A_IN_RANGE              : 9;
	__u32                                        : 7;

	__u32        u4VIDEO_A_OUT_RANGE             : 9;
	__u32                                        : 7;

	/* DWORD - 008*/
	__u32        fgOSD_SRC_SE                    : 1;
	__u32        fgOSD_SYNC_FLD_P                : 1;
	__u32        fgOSD_SYNC_H_P                  : 1;
	__u32        fgOSD_SYNC_V_P                  : 1;
	__u32        fgOSD_AUX_SRC_SE                : 1;
	__u32        fgOSD_AUX_SYNC_FLD_P            : 1;
	__u32        fgOSD_AUX_SYNC_H_P              : 1;
	__u32        fgOSD_AUX_SYNC_V_P              : 1;

	__u32        fgOSD_R_SRC_SE                  : 1;
	__u32        fgOSD_R_SYNC_FLD_P              : 1; /* only used in interlace output*/
	__u32        fgOSD_R_SYNC_H_P                : 1;
	__u32        fgOSD_R_SYNC_V_P                : 1;
	__u32                                        : 20;

	/* DWORD - 00C*/
	__u32        fgINIT_CRC_OSD1                 : 1;
	__u32        fgINIT_CRC_OSD2                 : 1;
	__u32        fgINIT_CRC_OSD3                 : 1;
	__u32        fgINIT_CRC_OSD4                 : 1;
	__u32        fgINIT_CRC_OSD5                 : 1;
	__u32        fgINIT_CRC_OSD6                 : 1;
	__u32        fgINIT_CRC_OSD7                 : 1;
	__u32        fgINIT_CRC_FMT_F                : 1;

	__u32        fgINIT_CRC_FMT_R                : 1;
	__u32        fgINIT_CRC_VDO_F                : 1;
	__u32        fgINIT_CRC_VDO_R                : 1;
	__u32        fgINIT_CRC_SCL                  : 1;
	__u32        fgINIT_CRC_FMT                  : 1;
	__u32        fgINIT_CRC_FPD                  : 1;
	__u32        fgINIT_CRC_TVE                  : 1;
	__u32        fgINIT_DVD_MIX                  : 1;

	__u32        fgCLR_CRC_OSD1                  : 1;
	__u32        fgCLR_CRC_OSD2                  : 1;
	__u32        fgCLR_CRC_OSD3                  : 1;
	__u32        fgCLR_CRC_OSD4                  : 1;
	__u32        fgCLR_CRC_OSD5                  : 1;
	__u32        fgCLR_CRC_OSD6                  : 1;
	__u32        fgCLR_CRC_OSD7                  : 1;
	__u32        fgCLR_CRC_FMT_F                 : 1;

	__u32        fgCLR_CRC_FMT_R                 : 1;
	__u32        fgCLR_CRC_VDO_F                 : 1;
	__u32        fgCLR_CRC_VDO_R                 : 1;
	__u32        fgCLR_CRC_SCL                   : 1;
	__u32        fgCLR_CRC_FMT                   : 1;
	__u32        fgCLR_CRC_FPD                   : 1;
	__u32        fgCLR_CRC_TVE                   : 1;
	__u32        fgCLR_DVD_MIX                   : 1;

	/* DWORD - 010*/
	__u32        u4CRC_OSD1                      : 16;
	__u32        u4CRC_OSD2                      : 16;

	/* DWORD - 014*/
	__u32        u4CRC_OSD3                      : 16;
	__u32        u4CRC_OSD4                      : 16;

	/* DWORD - 018*/
	__u32        u4CRC_LCPROC                    : 16;
	__u32        u4CRC_DVD_MIX                   : 16;

	/* DWORD - 01C*/
	__u32        u4CRC_OSD6                      : 16;
	__u32        u4CRC_OSD7                      : 16;

	/* DWORD - 020*/
	__u32        u4CRC_FMT_FRONT                 : 16;
	__u32        u4CRC_VDO_FRONT                 : 16;

	/* DWORD - 024*/
	__u32        u4CRC_FMT_REAR                  : 16;
	__u32        u4CRC_VDO_REAR                  : 16;

	/* DWORD - 028*/
	__u32        u4CRC_SCALER                    : 16;
	__u32        u4CRC_MIX                       : 16;

	/* DWORD - 02C*/
	__u32        u4CRC_FPD                       : 16;
	__u32        U4CRC_TVE                       : 16;

	/* DWORD - 030*/
	__u32        u4WRITE_BACK_SEL                : 2;
	__u32                                        : 2;
	__u32        u4WCH1_SRC_SEL                  : 2;
	__u32                                        : 1;
	__u32        fgWCH1_VDOIN_EN                 : 1;

	__u32        u4WCH2_SRC_SEL                  : 2;
	__u32                                        : 2;
	__u32        fgWCH2_VDOIN_EN                 : 1;
	__u32        fgWCH2_SERVO_DBG                : 1;
	__u32                                        : 18;

	/* DWORD - 034*/
	__u32        u4PANEL_HEND                    : 12;
	__u32                                        : 4;
	__u32        u4PANEL_HBGN                    : 12;
	__u32                                        : 4;

	/* DWORD - 038*/
	__u32        u4PANEL_VOEND                   : 11;
	__u32                                        : 5;
	__u32        u4PANEL_VOBGN                   : 11;
	__u32                                        : 5;

	/* DWORD - 03C*/
	__u32        u4PANEL_VEEND                   : 11;
	__u32                                        : 5;
	__u32        u4PANEL_VEBGN                   : 11;
	__u32                                        : 5;

	/* DWORD - 040*/
	__u32        fgDVD_SPU_MIX_EN                : 1;
	__u32        fgDVD_OSD_MIX_EN                : 1;
	__u32        fgAP_UI_MIX_EN                  : 1;
	__u32        fgY_BLACK_SEL                   : 1;
	__u32        fgV_TIMING_SEL                  : 1;
	__u32        fgDATA_OUTPUT_EN                : 1;
	__u32        fgWRITE_EN_INVERT               : 1;
	__u32                                        : 25;

	/* DWORD - 044*/
	__u32                                        : 32;

	/* DWORD - 048*/
	__u32        u4LAYER0_OFF_CR                 : 8;
	__u32        u4LAYER0_OFF_CB                 : 8;
	__u32        u4LAYER0_OFF_Y                  : 8;
	__u32                                        : 8;

	/* DWORD - 04C*/
	__u32        fgEN_CRC_OSD1_CLK               : 1;
	__u32        fgEN_CRC_OSD2_CLK               : 1;
	__u32        fgEN_CRC_OSD3_CLK               : 1;
	__u32        fgEN_CRC_OSD4_CLK               : 1;
	__u32        fgEN_CRC_LCPROC_CLK             : 1;
	__u32        fgEN_CRC_OSD6_CLK               : 1;
	__u32        fgEN_CRC_OSD7_CLK               : 1;
	__u32        fgEN_CRC_FMTF_CLK               : 1;
	__u32        fgEN_CRC_FMTR_CLK               : 1;
	__u32        fgEN_CRC_VDOF_CLK               : 1;
	__u32        fgEN_CRC_VDOR_CLK               : 1;
	__u32        fgEN_CRC_SCALER_CLK             : 1;
	__u32        fgEN_CRC_MIX_CLK                : 1;
	__u32        fgEN_CRC_FPD_CLK                : 1;
	__u32        fgEN_CRC_TVE_CLK                : 1;
	__u32        fgEN_DVD_MIX_CLK                : 1;
	__u32                                        : 16;

	/* DWORD - 050*/
	__u32         fgMIX_LAYER0_En                : 1;
	__u32         fgMIX_LAYER1_En                : 1;
	__u32         fgMIX_LAYER2_En                : 1;
	__u32         fgMIX_LAYER3_En                : 1;
	__u32         fgMIX_LAYER4_En                : 1;
	__u32         fgMIX_LAYER5_En                : 1;
	__u32                                        : 14;
	__u32         u4MIX_LAYER0_SEL               : 3;
	__u32                                        : 9;

	/* DWORD - 054*/
	__u32                                        : 32;

	/* DWORD - 058*/
	__u32                                        : 32;

	/* DWORD - 05C*/
	__u32                                        : 32;

	/* DWORD - 060*/
	__u32                                        : 32;

	/* DWORD - 064*/
	__u32                                        : 32;

	/* DWORD - 068*/
	__u32                                        : 32;

	/* DWORD - 06C*/
	__u32                                        : 32;

	/* DWORD - 070*/
	__u32                                        : 32;

	/* DWORD - 074*/
	__u32                                        : 32;

	/* DWORD - 078*/
	__u32                                        : 32;

	/* DWORD - 07C*/
	__u32                                        : 32;

	/* DWORD - 080*/
	__u32         fgEN_LCPROC_DATA               : 1;
	__u32         fgEN_LCPROC_TIMING             : 1;
	__u32                                        : 29;
	__u32         fgVDOUT_SYS_TEST_EN            : 1;
} PMX_HAL_MIX_FIELD_T;

typedef union _PMX_HAL_MIX_UNION_T {
	__u32              au4Reg[PMX_HAL_MIX_REG_NUM];
	PMX_HAL_MIX_FIELD_T    rField;
} PMX_HAL_MIX_UNION_T;


extern PMX_HAL_DISP_MAIN_UNION_T _rPmxHalMainSwReg;
extern PMX_HAL_DISP_AUX_UNION_T  _rPmxHalAuxSwReg;
extern PMX_HAL_MIX_UNION_T       _rPmxHalMixSwReg;

extern __u8 _rPmxDispMainRegMode[PMX_HAL_DISP_MAIN_REG_NUM];
extern __u8 _rPmxDispAuxRegMode[PMX_HAL_DISP_AUX_REG_NUM];
extern __u8 _rPmxMixRegMode[PMX_HAL_MIX_REG_NUM];

#endif  /* _PMX_HW_H_*/



