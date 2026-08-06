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
#ifndef _VDP_HW_H_
#define _VDP_HW_H_

#include "pmx_hw.h"
#include "x_lint.h"
#include "x_hal_io.h"
#include "x_hal_ic.h"
#include "chip_ver.h"
#include "drv_config.h"
#include "x_hal_ic.h"

/*#define VDP_DRAM800_SUPPORT*/

#ifdef __linux__

#ifndef IO_BASE_VA
#define IO_BASE_VA                      0xFD000000
#endif

#else
#define IO_BASE_VA                      0xA0000000
#endif

#define VDP_HAL_VDO_F_REG               (IO_BASE_VA + 0x42100)
#define VDP_HAL_VDO_R_REG               (IO_BASE_VA + 0x43100)

#define VDP_HAL_VDO_REG_NUM             (0x100/4)

typedef struct _HAL_VDO_FIELD_T {
	/* DWORD - 000 */
	__u32                                  : 5;
	__u32      u4Y_ADDR_Y                  : 24;
	__u32                                  : 3;

	/* DWORD - 004 */
	__u32                                  : 5;
	__u32      u4Y_ADDR_C                  : 24;
	__u32                                  : 3;

	/* DWORD - 008 */
	__u32      fgSWOFF                     : 1;
	__u32                                  : 4;
	__u32      u4X_ADDR_Y                  : 24;
	__u32                                  : 3;

	/* DWORD - 00C */
	__u32                                  : 5;
	__u32      u4X_ADDR_C                  : 24;
	__u32                                  : 3;

	/* DWORD - 010 */
	__u32      u4HBLOCK                    : 8;
	__u32      u4DW_NEED_SD                : 8;
	__u32      u4PIC_HEIGHT                : 11;
	__u32                                  : 1;
	__u32      u4HW_OPTION                 : 4;

	/* DWORD - 014 */
	__u32      u4VSCALE                    : 10;
	__u32                                  : 6;
	__u32      u4P_SKIP                    : 11;
	__u32                                  : 5;

	/* DWORD - 018 */
	__u32      u4Y_STAMBR                  : 7;
	__u32                                  : 1;
	__u32      u4C_STAMBR                  : 7;
	__u32                                  : 1;
	__u32      u4MAX_STAMBR                : 7;
	__u32                                  : 1;
	__u32      u4TOTAL_STAMBR              : 7;
	__u32                                  : 1;

	/* DWORD - 01C */
	__u32      fgVDOEN                     : 1;
	__u32      fgRACE                      : 1;
	__u32      fgYLAVG                     : 1;
	__u32      fgCLAVG                     : 1;
	__u32      fgHALF_Y                    : 1;
	__u32      fgHALF_C                    : 1;
	__u32      fgYLR                       : 1;
	__u32      fgCLR                       : 1;

	__u32      fgMPEG1                     : 1;
	__u32      fgCDG                       : 1;
	__u32      fgADDR_SW                   : 1;
	__u32      fgXHALF                     : 1;
	__u32      fgPF2                       : 1;
	__u32      fgAPF                       : 1;
	__u32      fgY2FF                      : 1;
	__u32      fgC2FF                      : 1;

	__u32      fgTFLD                      : 1;
	__u32      fgMI2P                      : 1;
	__u32      fgSTILL                     : 1;
	__u32      fgMOVE                      : 1;
	__u32      fgPD32_F                    : 1;
	__u32      fgDISP3                     : 1;
	__u32      fgPD32                      : 1;
	__u32      fgDEL_BK                    : 1;

	__u32      u4TST                       : 8;

	/* DWORD - 020 */
	__u32      u4YSLTT                     : 11;
	__u32                                  : 5;
	__u32      u4YSLBB                     : 11;
	__u32                                  : 5;

	/* DWORD - 024 */
	__u32      u4CSLTT                     : 11;
	__u32                                  : 5;
	__u32      u4CSLBB                     : 11;
	__u32                                  : 5;

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
	__u32      fgY_FRM                     : 1;
	__u32      fgC_FRM                     : 1;
	__u32      fgPFLD                      : 1;
	__u32      fgB_PIC                     : 1;
	__u32      fgFLDB                      : 1;
	__u32      fgAFLD                      : 1;
	__u32      fgUAFLD                     : 1;
	__u32      fgB_PIC2                    : 1;

	__u32      fgNDACT                     : 1;
	__u32      fgCRND                      : 1;
	__u32      fgMAOFF                     : 1;
	__u32                                  : 1;
	__u32      fgL1NF16                    : 1;
	__u32      fg1STSEL                    : 1;
	__u32                                  : 4;
	__u32      fgNPFOFF                    : 1;
	__u32                                  : 1;
	__u32      fgPSOFF                     : 1;
	__u32      fgYUV422                    : 1;
	__u32                                  : 1;
	__u32      fgCS_J                      : 1;

	__u32      fgYSLNG                     : 1;
	__u32      fgCSLNG                     : 1;
	__u32                                  : 6;

	/* DWORD - 034 */
	__u32      u4FIFO_LMT                  : 7;
	__u32                                  : 1;
	__u32      u4FIFO_INI                  : 8;

	__u32      u4HW_OPT                    : 8;
	__u32      u4FIFO_2ND                  : 7;
	__u32                                  : 1;

	/* DWORD - 038 */
	__u32                                  : 24;
	__u32      u4CTHRD                     : 7;
	__u32                                  : 1;

	/* DWORD - 03C */
	__u32      u4VDORST                    : 8;
	__u32                                  : 24;

	/* DWORD - 040 */
	__u32                                  : 32;

	/* DWORD - 044 */
	__u32                                  : 32;

	/* DWORD - 048 */
	__u32                                  : 32;

	/* DWORD - 04C */
	__u32                                  : 32;

	/* DWORD - 050 */
	__u32      u4YSLBT                     : 11;
	__u32                                  : 5;
	__u32      u4YSLTB                     : 11;
	__u32                                  : 5;

	/* DWORD - 054 */
	__u32      u4CSLBT                     : 11;
	__u32                                  : 5;
	__u32      u4CSLTB                     : 11;
	__u32                                  : 5;

	/* DWORD - 058 */
	__u32                                  : 24;
	__u32      u4CT_THRD                   : 8;

	/* DWORD - 05C */
	__u32                                  : 24;
	__u32      u4MTHRD                     : 8;

	/* DWORD - 060 */
	__u32      u4YX_CMB_CNT                : 20;
	__u32                                  : 12;

	/* DWORD - 064 */
	__u32      fgGET3F                     : 1;
	__u32                                  : 1;
	__u32      fgB3F                       : 1;
	__u32      fgOPT0                      : 1;
	__u32      fgGMLINE                    : 1;
	__u32      fgUMLINE                    : 1;
	__u32      fgNEW3F                     : 1;
	__u32      fgCMB_L                     : 1;

	__u32      fgNLS                       : 1;
	__u32      fgNLS2                      : 1;
	__u32      fgNEWTG                     : 1;
	__u32      fgOPT1                      : 1;
	__u32      fgYI2P                      : 1;
	__u32      fgCI2P                      : 1;
	__u32      fgFIX_SC                    : 1;
	__u32      fgSP_LD                     : 1;

	__u32      fgMBL_M2                    : 1;
	__u32      fgMB_F                      : 1;
	__u32      fgM_AVG                     : 1;
	__u32      fgF_STL                     : 1;
	__u32      fgOF_MO                     : 1;
	__u32      fgOR_MD                     : 1;
	__u32      fgIG_PK                     : 1;
	__u32      fgCMB_S                     : 1;

	__u32      fgS_CMB                     : 1;
	__u32      fgS_MD                      : 1;
	__u32      fgCC_SEL                    : 1;
	__u32      fgCMB_1F                    : 1;
	__u32      fgSTCMB                     : 1;
	__u32      fgC_CMB                     : 1;
	__u32      fgOPT2                      : 1;
	__u32      fgOPT3                      : 1;

	/* DWORD - 068 */
	__u32      u4M3F_TH                    : 8;
	__u32      u4M3F_LO                    : 8;
	__u32                                  : 12;
	__u32      u4US_CR_D                   : 2;
	__u32                                  : 1;
	__u32      fgMBL_S2                    : 1;

	/* DWORD - 06C */
	__u32      u4Y_STA_T                   : 5;
	__u32                                  : 3;

	__u32      u4Y_STA_B                   : 5;
	__u32                                  : 3;

	__u32      u4C_STA_T                   : 2;
	__u32      fgDL_MD                     : 1;
	__u32      fgDL_2ND                    : 1;
	__u32                                  : 4;

	__u32      u4C_STA_B                   : 2;
	__u32                                  : 6;

	/* DWORD - 070 */
	__u32      fgFLT_TW                    : 1;
	__u32      fgCF_TW                     : 1;
	__u32      fgM_EXP                     : 1;
	__u32      fgS_EXP                     : 1;
	__u32      fgNEW4F                     : 1;
	__u32      fgRW_MO                     : 1;
	__u32      u4OPT0_3                    : 4;
	__u32      fgU_MA4F                    : 1;
	__u32      fgOPT4                      : 1;
	__u32      fgNL_CS                     : 1;
	__u32      fgSEL_F3                    : 1;
	__u32      fgPF_S2                     : 1;
	__u32      fgCPF_S2                    : 1;

	__u32      u4OPT5_6                    : 2;
	__u32      fgC_LO_B                    : 1;
	__u32      u4OPT7_9                    : 3;
	__u32      fgCSCON                     : 1;
	__u32      fgCSCEN                     : 1;
	__u32      u4OPT10                     : 2;
	__u32      fgDEL_YL                    : 1;
	__u32      fgDEL_CL                    : 1;
	__u32      fgWM_SL                     : 1;
	__u32      u4OPT12                     : 3;

	/* DWORD - 074 */
	__u32      u4XZ_MOTION_C               : 20;
	__u32                                  : 12;

	/* DWORD - 078 */
	__u32      u4NEW_ADDR_SWAP             : 3;
	__u32      u4NEW_ADDR_SWAP_EN          : 1;
	__u32                                  : 19;
	__u32      u4HW_OPT_NEW                : 1;

	__u32                                  : 2;
	__u32      u4SPF_CNT                   : 1;
	__u32                                  : 1;
	__u32      u4SPF_LMT                   : 4;

	/* DWORD - 07C */
	__u32                                  : 5;
	__u32      fgGET_4FLD                  : 1;
	__u32      fgYMORE_FLD_ADDR            : 1;
	__u32      fgWXYZ_FLD_ADDR             : 1;

	__u32                                  : 3;
	__u32      fgBW_LIMIT                  : 1;
	__u32                                  : 4;

	__u32      fgOLD_ADDR_SWAP             : 1;
	__u32      fgBK_LINEEND_OFF            : 1;
	__u32      fgOLD_MBL_STA               : 1;
	__u32                                  : 1;
	__u32      fgY_I2P_NOFRM               : 1;
	__u32      fgC_I2P_NOFRM               : 1;
	__u32      fgHD_LRMN_OFF               : 1;
	__u32      fgRECOVER_OPT               : 1;

	__u32                                  : 8;

	/* DWORD - 080 */
	__u32                                  : 5;
	__u32      u4W_ADDR_Y                  : 24;
	__u32                                  : 3;

	/* DWORD - 084 */
	__u32                                  : 5;
	__u32      u4Z_ADDR_Y                  : 24;
	__u32                                  : 3;

	/* DWORD - 088 */
	__u32      u4FDIFF_TH3                 : 21;
	__u32                                  : 3;

	__u32      fgMA4F                      : 1;
	__u32      u4FDIFF_CTRL                : 3;
	__u32      fgBP_YC                     : 1;
	__u32      fgASAW                      : 2;
	__u32      fgMDDI_CLK_ON               : 1;

	/* DWORD - 08C */
	__u32      u4FDIFF_TH2                 : 21;
	__u32                                  : 3;
	__u32      u4MA_VIDEO_M                : 8;

	/* DWORD - 090 */
	__u32      u4FDIFF_TH1                 : 21;
	__u32                                  : 3;
	__u32      u4MA_HW_OPT1                : 8;

	/* DWORD - 094 */
	__u32      u4TH_MIN_XZ                 : 10;
	__u32                                  : 2;
	__u32      u4TH_MED_XZ                 : 10;
	__u32                                  : 2;
	__u32      u4MA_TST_MODE               : 8;

	/* DWORD - 098 */
	__u32      u4TH_NM_XZ                  : 10;
	__u32                                  : 2;
	__u32      u4TH_ED_XZ                  : 10;
	__u32                                  : 2;
	__u32      u4H_ED_TH                   : 8;

	/* DWORD - 09C */
	__u32      u4TH_MIN_YW                 : 9;
	__u32                                  : 3;
	__u32      u4TH_MED_YW                 : 9;
	__u32                                  : 3;
	__u32      u4SAW_TH                    : 8;

	/* DWORD - 0A0 */
	__u32      u4TH_NM_YW                  : 9;
	__u32                                  : 3;
	__u32      u4TH_ED_YW                  : 9;
	__u32                                  : 3;
	__u32      u4WH_TX_TH                  : 8;

	/* DWORD - 0A4 */
	__u32      u4FCH_MIN_XZ                : 10;
	__u32                                  : 2;
	__u32      u4FCH_NM_XZ                 : 10;
	__u32                                  : 2;
	__u32      u4VMV_FCH                   : 8;

	/* DWORD - 0A8 */
	__u32      u4FCH_MIN_YW                : 9;
	__u32                                  : 3;
	__u32      u4FCH_NM_YW                 : 9;
	__u32                                  : 1;
	__u32      u4CRM_LVL                   : 2;
	__u32      u4FR_LVL                    : 2;
	__u32      u4X_POS_ST                  : 6;

	/* DWORD - 0AC */
	__u32      u4CRM_SAW                   : 8;
	__u32      u4TV_LINE_ST                : 8;
	__u32      u4FD_CNT                    : 8;
	__u32      u4MA_QUALITY_MODE           : 8;

	/* DWORD - 0B0 */
	__u32      u4EDGE_P_TH                 : 8;
	__u32      u4EDGE_VERT_TH              : 8;
	__u32      u4EDGE_CROSS_TH             : 8;
	__u32      u4MA_EDGE_MODE              : 8;

	/* DWORD - 0B4 */
	__u32      u4EDGE_63D_TH               : 8;
	__u32      u4EDGE_ABS_GRAD_TH          : 8;
	__u32      u4EDGE_UD_RESTRICT_TH       : 8;
	__u32      u4MA_EDGE_ADV_CTRL          : 8;

	/* DWORD - 0B8 */
	__u32      u4EDGE_MULTI_EDGE_TH        : 8;
	__u32      u4EDGE_MEDGE_CNT_TH         : 4;
	__u32                                  : 4;
	__u32      u4EDGE_3LINE_GRAD_TH        : 8;
	__u32      u4MA_EDGE_MISC              : 8;

	/* DWORD - 0BC */
	__u32      u4EDGE_HOR_DIFF_TH          : 8;
	__u32      u4EDGE_EXP_TH               : 8;
	__u32      u4EDGE_V3_CTRL              : 8;
	__u32      u4EDGE_V3_QUAL              : 8;

	/* DWORD - 0C0 */
	__u32      fgBL_MODE                   : 4;
	__u32                                  : 15;
	__u32      fgSAW_5L                    : 1;
	__u32      fgMDEXP                     : 1;
	__u32      fgVT_BL                     : 1;
	__u32      fgOLD_SAW                   : 1;
	__u32                                  : 9;

	/* DWORD - 0C4 */
	__u32      u4PD_LINE_UNLIKE_TH         : 8;
	__u32      u4PD_LINE_UNLIKE_INTV       : 5;
	__u32                                  : 3;

	__u32      u4PD_COMB_TH                : 7;     /* comb counter threshold*/
	__u32                                  : 1;
	__u32      fgPD32NEW_EN                : 1;
	__u32                                  : 1;
	__u32      fgPD32NEW_CNT               : 1;
	__u32      fgVDO_PD_CTRL_MODE          : 5;

	/* DWORD - 0C8 */
	__u32                                  : 8;
	__u32      u4PD_BPF_THRD               : 8;
	__u32                                  : 8;
	__u32      fgPDULS                     : 1;
	__u32                                  : 7;

	/* DWORD - 0CC */
	__u32      u4PD_DST_START              : 8;
	__u32      u4PD_DST_END                : 8;
	__u32      u4SUBTITLE_THRD             : 8;
	__u32      fgSUBTITLE_ERASE_EN         : 1;
	__u32      fgSUBTITLE_REG_EN           : 1;
	__u32      fgSUBTITLE_REG_VDO          : 1;
	__u32                                  : 5;

	/* DWORD - 0D0 */
	__u32      fgCRM_3FMD                  : 1;
	__u32      fgCRM_FDIFF                 : 1;
	__u32      fgCRM_EXP_OFF               : 1;
	__u32                                  : 5;

	__u32      u4CRM_DIFF                  : 8;
	__u32      fgFDIFF_LMT                 : 1;
	__u32      fgFDIFF_SAW                 : 1;
	__u32      u4FDIFF_ADJ                 : 2;
	__u32                                  : 12;

	/* DWORD - 0D4 */
	__u32      u4AUTO_RST_WIDTH            : 31;
	__u32      fgAUTO_RST_EN               : 1;

	/* DWORD - 0D8 */
	__u32       fgSHADOW_EN                : 1;
	__u32       fgMOTION_READ_SE           : 1; /* motion read only register shadow mode enable*/
	__u32                                  : 29;
	__u32       fgSHADOW_TRIGER            : 1;

	/* DWORD - 0DC */
	__u32                                  : 32;

	/* DWORD - 0E0 */
	__u32      u4DW_NEED_HD                : 9;
	__u32                                  : 14;
	__u32      fgSC_LN                     : 1;

	__u32      fgHDEN                      : 1;
	__u32      fgDNFON                     : 1;
	__u32      fgNDMRE                     : 1;
	__u32      fgSHSRM                     : 1;
	__u32      fgS4_M                      : 1;
	__u32                                  : 2;
	__u32      fgFRC_P                     : 1;

	/* DWORD - 0E4 */
	__u32                                  : 32;

	/* DWORD - 0E8 */
	__u32                                  : 32;

	/* DWORD - 0EC */
	__u32                                  : 32;

	/* DWORD - 0F0 */
	__u32      u4FLD_WY_MOTION             : 20;
	__u32                                  : 12;

	/* DWORD - 0F4 */
	__u32      u4FLD_WX_COMB               : 20;
	__u32                                  : 12;

	/* DWORD - 0F8 */
	__u32      u4UR_RST                    : 8;
	__u32                                  : 24;

	/* DWORD - 0FC */
	__u32                                  : 5;
	__u32      u4Z_ADDR_C                  : 24;
	__u32                                  : 3;
} HAL_VDO_FIELD_T;

typedef union _HAL_VDO_UNION_T {
	__u32              au4Reg[VDP_HAL_VDO_REG_NUM];
	HAL_VDO_FIELD_T     rField;
} HAL_VDO_UNION_T;

#define VDP_1 0   /* Front*/
#define VDP_2 1   /* Rear*/

#define GET_VDP_HW_PTR(id, reg) \
	do { \
		if (id == VDP_1) { \
			reg = (HAL_VDO_UNION_T *) vdof_reg; \
		} else if (id == VDP_2) { \
			reg = (HAL_VDO_UNION_T *) vdor_reg; \
		} else { \
			return; \
		} \
	} while (0)

#define GET_VDP_PTR(id, reg, mode) \
	do { \
		if (id == VDP_1) { \
			reg = &_rVdp1SwReg; \
			mode = _rVdp1RegMode; \
		} else if (id == VDP_2) { \
			reg = &_rVdp2SwReg; \
			mode = _rVdp2RegMode; \
		} else { \
			return; \
		} \
	} while (0)

#define GET_VDP_PTR_REGION(id, reg, mode, src, out) \
	do { \
		if (id == VDP_1) { \
			reg = &_rVdp1SwReg; \
			mode = _rVdp1RegMode; \
			src = &_rVdp1SrcRegion; \
			out = &_rVdp1OutRegion; \
		} else if (id == VDP_2) { \
			reg = &_rVdp2SwReg; \
			mode = _rVdp2RegMode; \
			src = &_rVdp2SrcRegion; \
			out = &_rVdp2OutRegion; \
		} else { \
			return; \
		} \
	} while (0)

#define GET_PMX_DISP_PTR(id, reg, mode) \
	do { \
		if (id == VDP_1) { \
			reg.pDispMain = &_rPmxHalMainSwReg; \
			mode      = _rPmxDispMainRegMode; \
		} else if (id == VDP_2) { \
			reg.pDispAux  = &_rPmxHalAuxSwReg; \
			mode      = _rPmxDispAuxRegMode; \
		} else { \
			return; \
		} \
	} while (0)

#endif  /* _VDP_HW_H_*/









