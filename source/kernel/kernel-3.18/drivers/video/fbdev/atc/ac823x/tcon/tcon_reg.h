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
#ifndef _TCON_REG_H_
#define _TCON_REG_H_

#ifndef __ARM2__
#include <media/atc/display_inc.h>
#else
#include "display_inc.h"
#endif

#ifndef DWRD
#define DWRD DWORD
#endif

#define TCON_SUPPORT
#ifdef TCON_SUPPORT

/*==========================//*/
/*New Feature Support Define begin*/
#define FEATURE_GAMMA_TAB_READ  /*MT3363 add this feature*/
#define FEATURE_SCE_TAB_READ
#define FEATURE_COLORSPACE_INOUT_SEL
#define FEATURE_LC_DAT_READ    /* debug register for LC data read*/
#if MASTER_MODE_ENABLE
#define FEATURE_TCON_INPUTTIMING /*for tcon input timming feature*/
#endif
/*New Feature Support Define end*/
/*==========================//*/

/* **********************************************************************/
/* Panel TCON Related Registers define*/
/* **********************************************************************/
#define PCLRP_REG_OFFSET   0xA4400 /*0x2C00*/
#define PGMA_REG_OFFSET    0xA4700 /*0x2F00*/
#define PTCON_REG_OFFSET   0xA4800 /*0x3000*/

/* **********************************************************************/
/* Panel TCON Registers define*/
/* **********************************************************************/
/*Panel Color Proess: 0xa4400*/
#define RW_PCLRP_PTNGEN_L                   (0x00)
#define MLC_PTN_ENA        (0x01<<20)
#define MLC_PTN_STEP       (0x07<<24)
#define MLC_PTN_PREC       (0x03<<28)
#if defined(FEATURE_TCON_INPUTTIMING)
#define MLC_TIMINF_IN_EN   (0x01<<31)
#endif
#define MLC_PTN_ENA_SHF            (20)
#define MLC_PTN_STEP_SHF	   (24)
#define MLC_PTN_PREC_SHF	   (28)
#if defined(FEATURE_TCON_INPUTTIMING)
#define MLC_TIMINF_IN_EN_SHF (31)
#endif
#define RW_PCLRP_LUMA_Y                     (0x04)
#define DELAY_Y            (0x07<<0)
#define OFFSET1_Y          (0x1FF<<3)
#define OFFSET2_Y          (0x1FF<<12)
#define INV_Y              (0x1<<21)
#define FIX_Y_EN           (0x1<<22)
#define FIX_Y              (0x1FF<<23)
#define DELAY_Y_SHF        (0)
#define OFFSET1_Y_SHF      (3)
#define OFFSET2_Y_SHF      (12)
#define INV_Y_SHF	         (21)
#define FIX_Y_EN_SHF       (22)
#define FIX_Y_SHF	         (23)

#define RW_PCLRP_GAIN_Y                     (0x08)
#define MLC_GAIN_Y_EN      (0x01<<0)
#define MLC_GAIN_Y         (0x1FF<<4)
#define MLC_GAIN_Y_EN_SHF	 (0)
#define MLC_GAIN_Y_SHF	 (4)
#define RW_PCLRP_LUMA_SCECTRL               (0x10)
#define MLC_MOD_ENA        (0x01<<20)
#define MLC_MOD_TYPE       (0x01<<22)
#define MLC_MOD_LMT        (0xFF<<24)
#define MOD_MUL            (0x01<<22)
#define MOD_SUB            (0x00<<22)
#define MLC_MOD_LMT_SHF    (24)

#define RW_PCLRP_BRIGHT_CONT                (0x0C)
#define CONTRAST_GAIN      (0xFF)
#define BRIGHT_GAIN        (0xFF<<16)
#define CONTRINFLUM        (0x3F<<8)
#define CONTR_CORE         (0xFF<<24)
#define CONTRAST_GAIN_SHF  (0)
#define BRIGHT_GAIN_SHF    (16)
#define CONTRINFLUM_SHF    (8)
#define CONTR_CORE_SHF     (24)

#define RW_PCLRP_PTNGEN_C                   (0x18)
#define MLC_XFRONT         (0x01<<19) /* use to xor with input src Msb(so can chg Yc 2 Yuv)*/
#define MLC_PTN_ENA_U      (0x01<<20)
#define MLC_PTN_STEP_U     (0x07<<21)
#define MLC_PTN_PREC_U     (0x03<<24)
#define MLC_PTN_ENA_V      (0x01<<26)
#define MLC_PTN_STEP_V     (0x07<<27)
#define MLC_PTN_PREC_V     (0x03<<30)

#define MLC_XFRONT_SHF	 (19) /* use to xor with input src Msb(so can chg Yc 2 Yuv)*/
#define MLC_PTN_ENA_U_SHF  (20)
#define MLC_PTN_STEP_U_SHF (21)
#define MLC_PTN_PREC_U_SHF (24)
#define MLC_PTN_ENA_V_SHF	 (26)
#define MLC_PTN_STEP_V_SHF (27)
#define MLC_PTN_PREC_V_SHF (30)

#define RW_PCLRP_CHROMA_U                   (0x1C)
#define DELAY_U            (0x07)
#define OFFSET1_U          (0x1FF<<3)
#define OFFSET2_U          (0x1FF<<12)
#define INV_U              (0x1<<21)
#define FIX_U_EN           (0x1<<22)
#define FIX_U              (0x1FF<<23)
#define DELAY_U_SHF        (0)
#define OFFSET1_U_SHF      (3)
#define OFFSET2_U_SHF      (12)
#define INV_U_SHF          (21)
#define FIX_U_EN_SHF       (22)
#define FIX_U_SHF          (23)

#define RW_PCLRP_CHROMA_V                   (0x20)
#define DELAY_V            (0x07)
#define OFFSET1_V          (0x1FF<<3)
#define OFFSET2_V          (0x1FF<<12)
#define INV_V	             (0x1<<21)
#define FIX_V_EN           (0x1<<22)
#define FIX_V	             (0x1FF<<23)
#define DELAY_V_SHF        (0)
#define OFFSET1_V_SHF      (3)
#define OFFSET2_V_SHF      (12)
#define INV_V_SHF          (21)
#define FIX_V_EN_SHF       (22)
#define FIX_V_SHF          (23)

#define RW_PCLRP_GAIN_UV                    (0x24)
#define MLC_GAIN_U         (0x1FF<<4)
#define MLC_GAIN_V         (0x1FF<<20)
#define MLC_GAIN_U_EN      (0x01<<0)
#define MLC_GAIN_V_EN      (0x01<<16)
#define MLC_GAIN_U_SHF     (4)
#define MLC_GAIN_V_SHF     (20)
#define MLC_GAIN_U_EN_SHF	 (0)
#define MLC_GAIN_V_EN_SHF	 (16)

#define RW_PCLRP_SATURATION  (0x2C)
#define SAT_GAIN   (0xFF)
#define SAT_GAIN_SHF       (0)
/*0x00 (gain = 0) - 0x80 (gain = 1.0) - 0xFF (gain=2.0). Saturation gain = setting/0x80.*/
#define RW_PCLRP_HUE_SCECTRL                (0x30)
#define MLC_SCE_ENA        (0x01<<0)
#define SRAM_WR_MODE       (0x01<<1)
#define SRAM_RD_ENA        (0x01<<4)
#define HUE_GLOB_PREC      (0x01<<6)
#define HUE_DEGREE         (0x3F<<8)
#define SCE_UCLAMP         (0x7<<20)
#define SCE_VCLAMP         (0x7<<24)
#define MLC_SCE_ENA_SHF    (0)
#define SRAM_RD_ENA_SHF    (4)
#define HUE_GLOB_PREC_SHF  (6)
#define HUE_DEGREE_SHF     (8)
#define SCE_UCLAMP_SHF     (20)
#define SCE_VCLAMP_SHF     (24)

#define RW_PCLRP_HUE_SCE_CFG               (0x34)
#define HUE_DG_PREC        (0x01<<0)

#define RW_PCLRP_SCE_TABLE   (0x38)
#define MLC_LD_ENA         (0x01<<0)
#define HUE_DEGREE (0x3F<<8)

#define RW_PCLRP_SUPPRESSION                (0x40)
#define SPC_ENA            (0x01<<0)
#define SPC_LP_ENA         (0x01<<2)
#define SPC_GAIN           (0xFF<<4)
#define SPC_SEL            (0x7<<12)
#define SPCC_SEL           (0x7<<15)
#define SPC_LP_SEL         (0x3<<18)
#define SPC_SUB_DIV        (0x3<<20)
#define SEED_OFFSET        (0x1FF<<23)

#define SPC_ENA_SHF        (0)
#define SPC_LP_ENA_SHF     (2)
#define SPC_GAIN_SHF       (4)
#define SPC_SEL_SHF        (12)
#define SPCC_SEL_SHF       (15)
#define SPC_LP_SEL_SHF     (18)
#define SPC_SUB_DIV_SHF    (20)
#define SEED_OFFSET_SHF    (23)

#define RW_UV2CBCR_CONV                     (0x44)
#define MLC_V2CR_GAIN      (0x1FF<<20)
#define MLC_U2CB_GAIN      (0x1FF<<8)
#define MLC_UV_CONV_EN     (0x01<<0)
#define MLC_OUTFRONT       (0x01<<4)     /*signed /unsigned flag for  xor*/
#define MLC_V2CR_GAIN_SHF	 (20)
#define MLC_U2CB_GAIN_SHF  (8)
#define MLC_OUTFRONT_SHF   (4)
#define MLC_UV_CONV_EN_SHF (0)

#define RW_PCLRP_BLACK_EXTENSION            (0x4c)
#define BLEND_ENA		     (0x01<<31)
#define BLEV_EDIF		     (0x01<<20)
#define BLEND_SLOPE	     (0x3FF<<8)
#define BLEND_ANCHOR	     (0x3F<<0)
#define BLEND_ENA_SHF		 (31)
#define BLEV_EDIF_SHF		 (20)
#define BLEND_SLOPE_SHF          (8)
#define BLEND_ANCHOR_SHF	 (0)

#define RW_PCLRP_WHITE_EXTENSION            (0x50)
#define WLEND_ENA	         (0x01<<31)
#define WLEV_EDIF	         (0x01<<20)
#define WLEND_SLOPE	     (0x7FF<<8)
#define WLEND_ANCHOR	     (0x7F<<0)
#define WLEND_ENA_SHF		 (31)
#define WLEV_EDIF_SHF		 (20)
#define WLEND_SLOPE_SHF          (8)
#define WLEND_ANCHOR_SHF	 (0)

#define RW_PCLRP_CTI       (0x54)
#define CTI_T_SELECT       (0x07<<0)
#define CTI_GAIN_SHARP     (0x7f<<4)
#define CTI_DZONE          (0xF<<12)
#define CTI_MAX_MIN_JG     (0x1<<16)
#define CTI_PTADDSUB_INV   (0x1<<17)
#define CTI_LP_ENA         (0x1<<20)
#define CTI_LP_SEL         (0x3<<21)
#define CTI_T_SELECT_SHF     (0)
#define CTI_GAIN_SHARP_SHF   (4)
#define CTI_DZONE_SHF        (12)
#define CTI_MAX_MIN_JG_SHF   (16)
#define CTI_PTADDSUB_INV_SHF (17)
#define CTI_LP_ENA_SHF       (20)
#define CTI_LP_SEL_SHF       (21)

#define RW_PCLRP_CTI1                       (0x5c)
#define CTI_FIR_COEFF      (0xfffff<<8)
#define CTI_FIR_COEFF_SHF     (8)
#define RW_PCLRP_CTI2                       (0x60)
#define HD_AMP             (0xFF<<8)
#define HDDETECT_EN         (0x1<<16)
#define HD_AMP_SHF			(8)
#define HDDETECT_EN_SHF       (16)
#if defined(FEATURE_SCE_TAB_READ)
#define RW_PCLRP_SCE_TABLE_READ              (0x64)
#define MLC_SCE_READ_ENA      (0x1<<0)
#define MLC_SCE_READ_ADDR     (0x1FF<<1)
#define MLC_SCE_P_SAT         (0xFF<<10)
#define MLC_SCE_P_HUE         (0x3F<<18)
#define MLC_SCE_P_LUMA        (0xFF<<24)
#define MLC_SCE_READ_ENA_SHF   (0)
#define MLC_SCE_READ_ADDR_SHF  (1)
#define MLC_SCE_P_SAT_SHF      (10)
#define MLC_SCE_P_HUE_SHF      (18)
#define MLC_SCE_P_LUMA_SHF     (24)
#endif

/*0x2F00 GAMMA 0xB700*/
#define RW_PGMA_CTRL0      (0x00)
#define SOFT_RESET         (0x01<<0)  /*software reset 1:reset*/
#define ACCESS_GAMMA       (0x01<<1) /*enable RISC to access gamma*/
#define GAMMA_ON           (0x01<<2) /*gamma output enable*/
#define H_DELAY            (0x03<<4) /*delay horizontal counter for dither*/
#define FRONT_GAMMA        (0x01<<6) /*front Gamma (before Gain Offset)*/
#define FULL_RANGE_IN      (0x01<<7) /*3356 fix to support Full range colorspace convert*/
#define DR_MODE            (0x03<<8) /*dither output mode*/
#define RUN_DR_EN          (0x01<<10)
#define ERR_DR_EN          (0x01<<11)
#define FRAME_PHASE        (0x0F<<12)
#define ROUND_EN           (0x01<<16) /*rounding enable*/
#define LSB_OFF            (0x01<<17)
#define RGB_BIT_SEL        (0x03<<18)
#define FPD_RGB6BIT        (0x01<<18)
#define FPD_RGB8BIT        (0x01<<19)

#define FPD_RGBDRV         (0x03<<20)
#define FPD_RGBSR          (0x01<<22)
#define YUV_MOD_SEL        (0x01<<23) /*1:yuv709, 0-yuv601*/
#define YUV2RGB_EN         (0x01<<24)
#define BYPASS_GAMMA       (0x01<<25)
#define COLOR_BAR_EN       (0x01<<26)
#define COLOR_BAR_TYPE     (0x01<<27)
#define SIGNED_DATA        (0x01<<28)
#define TEST_MODEF         (0x07<<29)

#define FULL_RANGE_IN_SHF  (7)
#define DR_MODE_SHF        (8) /*dither output mode*/
#define LSB_OFF_SHF        (17)
#define YUV_MOD_SEL_SHF    (23)       /* 3356 fix to support 709 & 601 Mode Color Space Convert*/
#define YUV2RGB_EN_SHF     (24)
#define COLOR_BAR_EN_SHF   (26)
#define COLOR_BAR_TYPE_SHF (27)

#define RW_PGMA_CTRL1 (0x04)
#define R_GAMMA_MAX        (0xFF<<0)
#define G_GAMMA_MAX        (0xFF<<8)
#define B_GAMMA_MAX        (0xFF<<16)
#define COLOR_WIDTH        (0xFF<<24)
#define COLOR_WIDTH_SHF    (24)

#define RW_PGMA_GAIN (0x08)
#define R_GAIN      (0xFF<<0)
#define G_GAIN      (0xFF<<8)
#define B_GAIN      (0xFF<<16)

#define RW_PGMA_OFFSET0 (0x0C)
#define R_OFFSET      (0x3FFF<<0)
#define G_OFFSET      (0x3FFF<<16)

#define RW_PGMA_OFFSET1 (0x10)
#define B_OFFSET      (0x3FFF<<0)
#define RW_PGMA_R_GAMMA (0x14)
#define RW_PGMA_G_GAMMA (0x18)
#define RW_PGMA_B_GAMMA (0x1C)
#define GAMMA_ADDR      (0x3F<<0)
#define GAMMA_DATA      (0xFF<<16)
#define RW_PGMA_REV_CTRL    (0x20)
#define PGMA_REV_EN        (0x01<<31)
#define PGMA_REV_THD       (0x3F<<24)
#define RW_PGMA_CTRL4                       (0x28)
#define FULL_RANGE_OUT     (1<<4)
#define FULL_RANGE_OUT_SHF (4)
#define RW_PGMA_DAT_DBG                     (0x30)
#define DBG_DATA_B         (0x3FF<<0)
#define DBG_DATA_G         (0x3FF<<10)
#define DBG_DATA_R         (0x3FF<<20)
#define DBG_DATA_B_SHF     (0)
#define DBG_DATA_G_SHF     (10)
#define DBG_DATA_R_SHF     (20)
#define RW_PGMA_DITHER                      (0x80)
#define DITHER_IN_FORMAT   (0X03<<0)
#define DITHER_IN_8BIT     (0X00<<0)
#define DITHER_IN_10BIT    (0X01<<0)
#define DITHER_IN_12BIT    (0X02<<0)
#define DITHER_IN_14BIT    (0X03<<0)

#define DITHER_OUT_FORMAT  (0X03<<4)
#define DITHER_OUT_4BIT    (0X00<<4)
#define DITHER_OUT_6BIT    (0X01<<4)
#define DITHER_OUT_8BIT    (0X02<<4)
#define DITHER_OUT_10BIT   (0X03<<4)
#define OUT_FMT_SHF        (4)
#define DITHER_BYPASS      (0x01<<8)
#define GMA_DR_BYPASS      (0x01<<24)
#define LCPROC_BYPASS      (0x01<<25)
#define DITHER_NEW_EN      (0X1<<31)    /*switch between FRC dither & old dither*/
#define DITHER_BYPASS_SHF  (8)
#define GMA_DR_BYPASS_SHF  (24)
#define LCPROC_BYPASS_SHF  (25)
#define DITHER_NEW_EN_SHF  (31)    /*switch between FRC dither & old dither*/

#if defined(FEATURE_GAMMA_TAB_READ)
#define RW_PGMA_GMA_READ                  (0x9C)
#define GAMMA_READ_ADDR     (3F<<0)
#define GAMMA_READ_EN		  (1<<7)
#define GAMMA_READ_DAT_B    (0xFF<<8)
#define GAMMA_READ_DAT_G    (0xFF<<16)
#define GAMMA_READ_DAT_R    (0xFF<<24)
#define GAMMA_READ_DAT_B_SHF  (8)
#define GAMMA_READ_DAT_G_SHF  (16)
#define GAMMA_READ_DAT_R_SHF  (24)
#endif
/*0x3000 TCON*/
#define RW_PTCON_GLB0  (0x00)
#define TCON_EN  (0x1<<0)
#define TCKA_EN              (0x1<<1)
#define TCON_EN_SHF            (0)
#define TCKA_EN_SHF            (1)
#define RW_PTCON_GLB1  (0x04)
#define TCON_OUT0_EN         (0x1<<0)
#define TCON_OUT0_EN_SHF		 (0)
#define RW_PTCON_OUT0_H                     (0x10)
#define TCON_OUT0_HS         (0x7FF<<0)
#define TCON_OUT0_HE         (0x7FF<<12)
#define TCON_OUT0_HS_SHF         (0)
#define TCON_OUT0_HE_SHF         (12)
#define RW_PTCON_OUT0_V                     (0x14)
#define TCON_OUT0_VS		 (0x7FF<<0)
#define TCON_OUT0_VE		 (0x7FF<<12)
#define TCON_OUT0_VS_SHF         (0)
#define TCON_OUT0_VE_SHF         (12)
#define RW_PTCON_GLB2  (0x08)
#define RW_PTCON_GLB3  (0x0C)
#define RW_PTCON_TIM1_H  (0x20)
#define RW_PTCON_TIM1_V  (0x24)
#define RW_PTCON_TIM2_H  (0x30)
#define RW_PTCON_TIM2_V  (0x34)
#define RW_PTCON_TIM8_H  (0x90)
#define RW_PTCON_TIM8_V  (0x94)
#define TCON_TnHS (0x7FF<<0)
#define TCON_TnHE (0x7FF<<12)
#define TCON_TnVS (0x7FF<<0)
#define TCON_TnVE (0x7FF<<12)
#define RW_PTCON_FPD_CFG                    (0xE0)
#define TCON_LR              (0x1<<13)
#define TCON_UD              (0x1<<14)

#define TCON_HSYNC_POL_INV   (0x1<<16)
#define TCON_VSYNC_POL_INV   (0x1<<17)
#define TCON_ADJ_DE_SEL      (0x1<<19)
#define TCON_ADJ_HSYNC_SEL   (0x1<<20)
#define TCON_ADJ_VSYNC_SEL   (0x1<<21)

/*#define RW_PTCON_GLB4  (0x04)*/
/*#define TCON_UPD  (0x1 << 4)*/
/*for check register value*/
#define MAX_1BIT    (0X1)
#define MAX_2BIT    (0X3)
#define MAX_3BIT    (0X7)
#define MAX_6BIT    (0X3F)
#define MAX_8BIT    (0XFF)
#define MAX_9BIT    (0X1FF)
#define MAX_10BIT   (0X3FF)

#define DIGITAL_PANEL
/*///////////////////////////////////////////////////////////////////////////////////////////////////*/
#ifndef DIGITAL_PANEL
#define PANEL_AUO
/*#define PANEL_LG*/
/*#define PW090XS2  //640*234*/
#else
#define   PANEL_AUO_102_DIGITAL
/* #define PANEL_SHARP_104_DIGITAL*/
/*#define PANEL_PVI_DIGITAL*/
#if defined(PANEL_PVI_DIGITAL) || defined(PANEL_AUO_102_DIGITAL) || defined(PANEL_SHARP_104_DIGITAL)
#define PANEL_DIGITAL_6BIT
/*  #define PANEL_DIGITAL_8BIT   // by sxj*/
#endif
#endif /* DIGITAL_PANEL*/
typedef enum {
	CLRP_SCE_TABLE,    /*sce data upt*/
	CLRP_SCE_ONOFF,    /* on off select*/
	CLRP_SCE_LUMATYPE, /*luma gain or add*/
	CLRP_SCE_TEST,     /*fix table*/
	CLRP_SCE_HUE_GLOBAL,
	CLRP_SCE_SETTING_CFG,
	CLRP_SCE_GET_TABLE,
	CLRP_SCE_USER      /* user table*/

} CP_SCE_CMD_T;

typedef enum {
	CLRP_CONTR_GAIN,
	CLRP_CONTR_BRIT,
	CLRP_CONTR_SATR,
	CLRP_CONTR_INFLUM,
	CLRP_CONTR_CORE,
	CLRP_CONTR_RESET
} CLRP_CONTR_CMD_T;

typedef enum {
	BLACK_LEVEL_EX,
	WHITE_LEVEL_EX,
	BW_LEVEL_EX_OFF
} CLRP_BW_EX_CMD_T;

typedef enum {
	CLRP_GPP_DELAY,
	CLRP_GPP_OFFSET1,
	CLRP_GPP_OFFSET2,
	CLRP_GPP_GAIN,
	CLRP_GPP_INV,
	CLRP_GPP_FIX,
	CLRP_GPP_RST

} CLRP_GPP_PROC_CMD_T;


typedef enum {
	CLRP_Y_CHANEL,
	CLRP_U_CHANEL,
	CLRP_V_CHANEL,
	CLRP_ALL_CHANEL
} CLRP_YUV_CHAENL_SEL_T;

typedef enum {
	MIX_ENBALE,
	MIX_LAYEE_ORDER,
	MIX_DEST_LAYER_SEL,
	MIX_VDO_SRC_SEL
} VDO_MIX_CMD_T;

typedef enum {
	FPD_RGB_GAIN,            /* 0*/
	FPD_RGB_OFFSET,
	FPD_RGB_GAMMA_ON_OFF,
	FPD_RGB_GAMMA_TABLE	 /* 3*/
} GMA_RGB_PROC_CMD_T;

#ifdef PANEL_AUO /*A070FW03/ AT070TN01/ PW070XU3/ CLAA070WA03*/
const DWRD _TCON[32] = {
#ifdef TCON_HCK23_OUTPUT
	0x0000003C, 0x0104007E, 0x0000F51A, 0x00053FBF,
#else
	0x0000003C, 0x0104007E, 0x0000F51A, 0x00050F3F,
#endif
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00052051, 0x00000001, 0x00000000, 0x00000000,
	0x01006237, 0x00000001, 0x00000000, 0x00000000,
	0x00012228, 0x00000001, 0x00000000, 0x00000000,
	0x28000220, 0x00014012, 0x00000000, 0x00000000,
	0x00237206, 0x00000001, 0x00000000, 0x00000000,
	0x2A000001, 0x00000001, 0x00000000, 0x00000000
};
#elif defined(PANEL_LG)  /*LG-LB070W02*/
const DWRD _TCON[32] = {
	0x0200003c, 0x0304007F, 0x0020EF2F, 0x00050D7F,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00052051, 0x00000001, 0x00000000, 0x00000000,
	0x00002234, 0x00000001, 0x00000000, 0x00000000,
	0x00038019, 0x00000001, 0x00000000, 0x00000000,
	0x2001E015, 0x00013012, 0x00000000, 0x00000000,
	0x01054234, 0x00000001, 0x00000000, 0x00000000,
	0x2A000001, 0x00000001, 0x00000000, 0x00000000
};
#elif defined(PANEL_TMD) /*TMD-LTA070B343A*/
const DWRD _TCON[32] = {
	0x00000015, 0x0306067F, 0x0000F51A, 0x00050F7F,
	0x00004050, 0x00000001, 0x00000000, 0x00000000,
	0x00052051, 0x000FC012, 0x00000000, 0x00000000,
	0x0004C004, 0x000FF010, 0x00000000, 0x00000000,
	0x0004C22B, 0x00000001, 0x00000000, 0x00000000,
	0x2800016A, 0x0001100F, 0x00000000, 0x00000000,
	0x0022F21F, 0x000FB011, 0x00000000, 0x00000000,
	0x38000028, 0x00000001, 0x00000000, 0x00000000
};
#elif defined(PW090XS2) /**/
const DWRD _TCON[32] = {
	0x0000003C, 0x0104007E, 0x0000F51A, 0x00050D3F,/*0x00053FBF,*/
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00056055, 0x00000001, 0x00000000, 0x00000000,
	0x01016318, 0x00000001, 0x00000000, 0x00000000,
	0x00012228, 0x00000001, 0x00000000, 0x00000000,
	0x28000220, 0x00015013, 0x00000000, 0x00000000,
	0x00237206, 0x00000001, 0x00000000, 0x00000000,
	0x2A000001, 0x00000001, 0x00000000, 0x00000000
};
#elif defined(PANEL_PVI_DIGITAL) /*PM070WX*/
const DWRD _TCON[32] = {
	/*00*/ 0x003f0535, 0x0104007e, 0x0020c53f, 0x00010f7f,
	/*10*/ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/*20*/ 0x0008C08A, 0x00000001, 0x00000000, 0x00000000,
	/*30*/ 0x0004F04D, 0x00000001, 0x00000000, 0x00000000,
	/*40*/ 0x00051010, 0x00000001, 0x00000000, 0x00000000,
	/*50*/ 0x003e8001, 0x0000b00a, 0x00000000, 0x00000000,
	/*60*/ 0x0001F001, 0x00000015, 0x00000000, 0x00000000,
	/*70*/ 0x2A000014, 0x00000001, 0x00000000, 0x00000000
};
#elif defined(PANEL_AUO_102_DIGITAL) /*AUO102VW01*/
const DWRD _TCON[32] = {
	/*00*/ 0x003f0435, 0x0001007e, 0x0020c53f, 0x00010f7f,
	/*10*/ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
#ifdef PANEL_OVERSCAN_H
	/*20*/ 0x0008B08A , 0x00000001, 0x00000000, 0x00000000,
#else
	/*20*/0x00093091 , 0x00000001, 0x00000000, 0x00000000,
#endif
	/*30*/ 0x0004F04D, 0x00000001, 0x00000000, 0x00000000,
	/*40*/ 0x00051010, 0x00000001, 0x00000000, 0x00000000,
#ifdef PANEL_OVERSCAN_H
	/*50*/ 0x003e8001, 0x00019018, 0x00000000, 0x00000000,
#else
	/*50*/ 0x003e8001,
#ifdef PANEL_1024_600
	0x0001E01D,
#else
	0x00013012,
#endif
	0x00000000, 0x00000000,
#endif
	/*60*/ 0x0001F001, 0x00000015, 0x00000000, 0x00000000,
	/*70*/ 0x2A000014, 0x00000001, 0x00000000, 0x00000000
};
#elif defined(PANEL_SHARP_104_DIGITAL) /*LQ104V1DG51*/
const DWRD _TCON[32] = {
	/*00*/ 0x003f0635, 0x0203007e, 0x00000000, 0x00050C00,
	/*10*/ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/*20*/ 0x00093091, 0x00000001, 0x00000000, 0x00000000,
	/*30*/ 0x01004001, 0x00000001, 0x00000000, 0x00000000,
	/*40*/ 0x01000001, 0x00000001, 0x00000000, 0x00000000,
	/*50*/ 0x003e8001, 0x0000b00a, 0x00000000, 0x00000000,
	/*60*/ /*0x0001F001, 0x00000015, 0x00000000, 0x00000000,*/
	/*VSYNC*/0x01000001, 0x00003001, 0x00000000, 0x00000000,
	/*70*/ 0x2A000014, 0x00000001, 0x00000000, 0x00000000
};

#endif

const DWRD _pdTestSCETable[360] = { /*red param is modified.*/
	0x200020, 0x200020, 0x200020, 0x200020, 0x200020, 0x200020, 0x200020, 0x200020,
	0x200020, 0x200021, 0x200021, 0x200022, 0x200023, 0x200023, 0x200024, 0x200025,
	0x200026, 0x200027, 0x200028, 0x200029, 0x20002a, 0x20002b, 0x20002c, 0x20002d,
	0x20002e, 0x20002f, 0x20002f, 0x200030, 0x200030, 0x200030, 0x200030, 0x200030,
	0x200030, 0x200030, 0x200030, 0x200030, 0x200031, 0x200032, 0x200032, 0x200033,
	0x200034, 0x200033, 0x200032, 0x200031, 0x20002f, 0x20002d, 0x20002b, 0x200028,
	0x200024, 0x200020, 0x20001b, 0x200019, 0x200017, 0x200017, 0x200017, 0x200017,
	0x200018, 0x200019, 0x20001a, 0x20001a, 0x20001a, 0x20001b, 0x20001b, 0x20001c,
	0x20001d, 0x20001d, 0x20001e, 0x20001d, 0x20001d, 0x20001d, 0x20001d, 0x20001d,
	0x20001c, 0x20001c, 0x20001c, 0x20001b, 0x20001b, 0x20001b, 0x20001c, 0x20001c,
	0x20001c, 0x20001c, 0x20001c, 0x20001c, 0x20001c, 0x20001c, 0x20001c, 0x20001c,
	0x20001c, 0x20001b, 0x20001b, 0x20001b, 0x20001b, 0x20001a, 0x20001a, 0x20001a,
	0x20001a, 0x200019, 0x200019, 0x200019, 0x200018, 0x200018, 0x200018, 0x200018,
	0x200017, 0x200017, 0x200017, 0x200017, 0x200017, 0x200016, 0x200016, 0x200017,
	0x200017, 0x200017, 0x200017, 0x200018, 0x200018, 0x200018, 0x200019, 0x200019,
	0x200019, 0x200019, 0x20001a, 0x20001a, 0x20001a, 0x20001a, 0x20001a, 0x200019,
	0x200019, 0x200019, 0x200019, 0x200019, 0x200018, 0x200018, 0x200018, 0x200018,
	0x200017, 0x200017, 0x200017, 0x200017, 0x200017, 0x200017, 0x200017, 0x200017,
	0x200016, 0x200016, 0x200016, 0x200016, 0x200016, 0x200015, 0x200015, 0x200015,
	0x200014, 0x200014, 0x200014, 0x200013, 0x200013, 0x200013, 0x200012, 0x200012,
	0x200012, 0x200012, 0x200011, 0x200010, 0x20000f, 0x20000e, 0x20000e, 0x20000e,
	0x20000e, 0x200010, 0x200012, 0x200012, 0x200012, 0x200012, 0x200013, 0x200013,
	0x200013, 0x200013, 0x200013, 0x200013, 0x200013, 0x200014, 0x200014, 0x200014,
	0x200014, 0x200015, 0x200015, 0x200015, 0x200015, 0x200016, 0x200016, 0x200017,
	0x200017, 0x200017, 0x200018, 0x200018, 0x200019, 0x200019, 0x20001a, 0x20001a,
	0x20001b, 0x20001b, 0x20001b, 0x20001c, 0x20001c, 0x20001d, 0x20001d, 0x20001d,
	0x20001e, 0x20001e, 0x20001f, 0x20001f, 0x200020, 0x200020, 0x200021, 0x200021,
	0x200022, 0x200022, 0x200022, 0x200022, 0x200022, 0x200022, 0x200022, 0x200022,
	0x200021, 0x200021, 0x200021, 0x200021, 0x200022, 0x200022, 0x200023, 0x200024,
	0x200025, 0x200026, 0x200027, 0x200028, 0x200029, 0x20002a, 0x20002b, 0x20002c,
	0x20002d, 0x20002d, 0x20002e, 0x20002f, 0x20002f, 0x200030, 0x200030, 0x200031,
	0x200030, 0x200030, 0x20002f, 0x20002e, 0x20002e, 0x20002d, 0x20002d, 0x20002c,
	0x20002b, 0x20002b, 0x20002b, 0x20002c, 0x20002c, 0x20002d, 0x20002e, 0x20002e,
	0x20002f, 0x200030, 0x200031, 0x200031, 0x200031, 0x200032, 0x200032, 0x200032,
	0x200032, 0x200032, 0x200033, 0x200033, 0x200034, 0x200036, 0x200037, 0x200039,
	0x20003b, 0x20003d, 0x20003f, 0x200041, 0x200043, 0x200044, 0x200046, 0x200048,
	0x20004a, 0x20004d, 0x20001b, 0x200019, 0x200017, 0x200016, 0x200016, 0x200016,
	0x200016, 0x200017, 0x200019, 0x20001a, 0x20001c, 0x20001e, 0x200020, 0x200022,
	0x200024, 0x200025, 0x200026, 0x200026, 0x200027, 0x200027, 0x200027, 0x200028,
	0x200028, 0x200028, 0x200028, 0x200028, 0x200029, 0x200029, 0x200029, 0x200029,
	0x200029, 0x20002a, 0x20002a, 0x200029, 0x200029, 0x200029, 0x200029, 0x200028,
	0x200028, 0x200027, 0x200026, 0x200025, 0x200023, 0x200022, 0x200020, 0x20001e,
	0x20001c, 0x20001b, 0x200019, 0x200018, 0x200016, 0x200016, 0x200016, 0x200016,
	0x200016, 0x200016, 0x200017, 0x200017, 0x200018, 0x200019, 0x200019, 0x20001a,
	0x20001b, 0x20001c, 0x20001c, 0x20001d, 0x20001e, 0x20001e, 0x20001f, 0x200020,
};


const __u8 _pbPanelGamma[3][64] = {
	{
		0  , 39 ,  52, 68 ,  77, 84 ,  91,  97,
		102, 108, 112, 117, 121, 125, 129, 133,
		137, 140, 144, 147, 150, 153, 156, 159,
		162, 165, 167, 170, 173, 175, 178, 180,
		182, 185, 187, 189, 192, 194, 196, 198,
		200, 202, 204, 206, 208, 210, 212, 214,
		216, 218, 220, 222, 223, 225, 227, 229,
		230, 232, 234, 235, 237, 239, 240, 242
	},
	{
		0  , 39 ,  52, 68 ,  77, 84 ,  91,  97,
		102, 108, 112, 117, 121, 125, 129, 133,
		137, 140, 144, 147, 150, 153, 156, 159,
		162, 165, 167, 170, 173, 175, 178, 180,
		182, 185, 187, 189, 192, 194, 196, 198,
		200, 202, 204, 206, 208, 210, 212, 214,
		216, 218, 220, 222, 223, 225, 227, 229,
		230, 232, 234, 235, 237, 239, 240, 242
	},
	{
		0  , 0  ,  0 , 0 ,   28,  39,  47,  53,
		59 , 64 ,  69,  73,  77,  81,  84,  88,
		91 , 94 ,  97, 100, 103, 105, 108, 111,
		113, 116, 118, 120, 122, 125, 127, 129,
		131, 133, 135, 137, 139, 141, 143, 145,
		146, 148, 150, 152, 153, 155, 157, 158,
		160, 162, 163, 165, 167, 169, 173, 177,
		181, 185, 189, 193, 197, 201, 205, 209
	},
};

const __u8 _pbPanelGain[16] = {
	0x10, 0x20, 0x30, 0x40,
	0x50, 0x60, 0x70, 0x80,
	0x90, 0xa0, 0xb0, 0xc0,
	0xd0, 0xe0, 0xf0, 0xff
};

const DWRD _pdPanelOffset[16] = {
	0xF900, 0xFA00, 0xFB00, 0xFC00,
	0xFD00, 0xFE00, 0xFF00, 0x0000,
	0x0100, 0x0200, 0x0300, 0x0400,
	0x0500, 0x0600, 0x0700, 0x0800,
};


#endif /* TCON_SUPPORT*/

#endif /* _TCON_REG_H_*/



