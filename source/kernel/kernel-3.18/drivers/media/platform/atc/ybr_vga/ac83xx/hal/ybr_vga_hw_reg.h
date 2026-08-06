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


#ifndef YPBPR_VGA_HW_REG__
#define YPBPR_VGA_HW_REG__


#include <generated/atc_project.h>
#include <asm/io.h>
#include "vga_hal_io.h"
#include "x_hal_ic.h"


#ifdef CONFIG_ATC_PLATFORM_ac83xx
//ybr/vga hw register digital base
#define YBR_DIG_BASE (0xFD000000U + 0x22000U)
#else
#define IO_PHY_ADDR 0x10000000
#define IO_PHYS_SIZE 0x00400000

extern unsigned long IO_BASE_REG_VA;
//ybr/vga hw register digital base
#define YBR_DIG_BASE (IO_BASE_REG_VA + 0x22000U)
#endif


/**
Sync_Proc Register
*/
#define ASYNC_00 (YBR_DIG_BASE + 0x00U)
    #define AS_VSYNC_ACT_SEL (u32)(0x1U << 31) //[31:31]
    #define AS_CSYNC_DGLITCH_SEL (u32)(0x1U << 30) //[30:30]
    #define AS_HSYNC_DGLITCH_SEL (u32)(0x1U << 29)//[29:29]
    #define AS_VSYNC_DGLITCH_SEL (u32)(0x1U << 28)//[28:28]
    #define AS_SYNC_SEL (u32)(0x3U << 26)//[27:26]
    #define AS_HSYNC_IN_RST_POL (u32)(0x1U << 25)//[25:25]
    #define AS_ACT_IRQ_SEL (u32)(0x7U << 22)//[24:22]
    #define AS_CSYNC_INV (u32)(0x1U << 21)//[21:21]
    #define AS_HSYNC_INV (u32)(0x1U << 20)//[20:20]
    #define AS_VSYNC_INV (u32)(0x1U << 19)//[19:19]
    #define AS_VSYNC_DELAY_SEL (u32)(0x1U << 18)//[18:18]
    #define AS_AUTO_INVP (u32)(0x1U << 17)//[17:17]
    #define AS_MODE_CHG_CLEAR (u32)(0x1U << 16)//[16:16]
    #define AS_MODE_CHG_IRQ_SEL (u32)(0x3U << 14)//[15:14]
    #define AS_MOD_CHG_V (u32)(0x3FU << 8)//[13:8]
    #define AS_VLINCNT_ADD1_EN (u32)(0x1U << 6)//[6:6]
    #define AS_MOD_CHG_H (u32)(0x3FU)//[5:0]
    
#define ASYNC_01 (YBR_DIG_BASE + 0x04U)
    #define AS_DE_COMP_DIFF_TH (u32)(0xFFU << 24)//[31:24]
    #define AS_DE_COMP_SEL (u32)(0x3U << 22)//[23:22]
    #define ORI_DECOMPOSITE        (u32)0
    #define NEW_DECOMPOSITE1    (u32)2
    #define NEW_DECOMPOSITE2    (u32)1
    #define AS_CSYNC_CONT_THU (u32)(0x3FFU << 12)//[21:12]
    #define AS_DE_COMP_TOSH (u32)(0x1U << 10)//[10:10]
    #define AS_CSYNC_CONT_THL (u32)(0x3FFU)//[9:0]
    
#define ASYNC_02 (YBR_DIG_BASE + 0x08U)
    #define AS_DBG_OUT_SEL (u32)(0xFU << 28)//[31:28]
    #define AS_SP2_EN (u32)(0x1U << 27)//[27:27]
    #define AS_HLEN_VLEN_RESET_SP2 (u32)(0x1U << 26)//[26:26]
    #define AS_LONG_LENGTH_STATUS_HOLD (u32)(0x1U << 25)//[25:25]
    #define AS_HLEN_VLEN_RESET (u32)(0x1U << 24)//[24:24]
    #define AS_CSYNC_CONT_THH (u32)(0x3FFU << 12)//[21:12]
    #define AS_VACT_MP_TH (u32)(0x3FFU)//[9:0]
    
#define ASYNC_03 (YBR_DIG_BASE + 0x0CU)
    #define AS_VSYNC_MASK_TEST (u32)(0x1U << 31)//[31:31]
    #define AS_FLD_DET_1152I_EN (u32)(0x1U << 30)//[30:30]
    #define AS_AUTO_HV_SKEW_CLK27_EN (u32)(0x1U << 29)//[29:29]
    #define AS_CLEAR_AUTO_DELAY_FLAG (u32)(0x1U << 28)//[28:28]
    #define AS_FLD_DET_AUTO (u32)(0x1U << 27)//[27:27]
    #define AS_FLD_DET_OLD (u32)(0x1U << 26)//[26:26]
    #define AS_FIELD_DEF_INV (u32)(0x1U << 25)//[25:25]
    #define AS_FLD_DETECT_E (u32)(0xFFFU << 12)//[23:12]
    #define AS_FLD_DETECT_S (u32)(0xFFFU)//[11:0]
    
#define ASYNC_04 (YBR_DIG_BASE + 0x10U)
    #define AS_NEW_HSYNC_RST_EN (u32)(0x1U << 31)//[31:31]
    #define AS_HLEN_USE_ACTIVE (u32)(0x1U << 30)//[30:30]
    #define AS_NEW_HSYNC_RST_RANGE (u32)(0xFU << 24)//[27:24]
    #define AS_HLEN2_E (u32)(0xFFFU << 12)//[23:12]
    #define AS_HLEN2_S (u32)(0xFFFU)//[11:0]
#define ASYNC_05 (YBR_DIG_BASE + 0x14)
    #define AS_CH_DIFF_TH (u32)(0x3FU << 24)//[29:24]
    #define AS_CHRANGE_L (u32)(0xFFFU << 12)//[23:12]
    #define AS_CHRANGE_U (0xFFFU)//[11:0]
#define ASYNC_06 (YBR_DIG_BASE + 0x18U)
    #define AS_CV_DIFF_TH (u32)(0x3FU << 24)//[29:24]
    #define AS_CVRANTH_L (u32)(0xFFFU << 12)//[23:12]
    #define AS_CVRANGE_U (u32)(0xFFFU)//[11:0]
#define ASYNC_07 (YBR_DIG_BASE + 0x1CU)
    #define AS_VSYNC_OUT_HYST_THR (u32)(0xFFU << 24)//[31:24]
    #define AS_CV_STABLE_TH (u32)(0xFFFU << 12)//[23:12]
    #define AS_CH_STABLE_TH (u32)(0xFFFU)//[11:0]
#define ASYNC_08 (YBR_DIG_BASE + 0x20U)
    #define AS_VMASK1_OFF (u32)(0x1U << 31)//[31:31]
    #define AS_VMASK2_OFF (u32)(0x1U << 30)//[30:30]
    #define AS_VMASK3_OFF (u32)(0x1U << 29)//[29:29]
    #define AS_MASK_SLICE_EN (u32)(0x1U << 25)//[25:25]
    #define AS_MASK_SLICE_INV (u32)(0x1U << 24)//[24:24]
    #define AS_MV_HEND (u32)(0xFFFU << 12)//[23:12]
    #define AS_MV_HSTART (u32)(0xFFFU)//[11:0]
#define ASYNC_09 (YBR_DIG_BASE + 0x24U)
    #define AS_SERR_MASK_END (u32)(0xFFU << 24)//[31:24]
    #define AS_SERR_MASK_ST (u32)(0xFFU << 16)//[23:16]
    #define AS_MV_VACTIVE_END (u32)(0xFFU << 8)//[15:8]
    #define AS_MV_VACTIVE_ST (u32)(0xFFU)//[7:0]
#define ASYNC_0A (YBR_DIG_BASE + 0x28U)
    #define AS_VMASK1_ST (u32)(0xFFU << 24)//[31:24]
    #define AS_VMASK1_END (u32)(0xFFU << 16)//[23:16]
    #define AS_VMASK2_ST (u32)(0xFFU << 8)//[15:8]
    #define AS_VMASK2_END (u32)(0xFFU)//[7:0]
#define ASYNC_0B (YBR_DIG_BASE + 0x2CU)
    #define AS_C_DEGLITCH (u32)(0x3FU << 26)//[31:26]
    #define AS_H_DEGLITCH (u32)(0x7U << 23)//[25:23]
    #define AS_V_DEGLITCH (u32)(0x7U << 20)//[22:20]
    #define AS_MASK_SLICE_POS_SEL (u32)(0x1U << 16)//[16:16]
    #define AS_MASK_SLICE_END (u32)(0xFFU << 8)//[15:8]
    #define AS_MASK_SLICE_ST (u32)(0xFFU)//[7:0]
#define ASYNC_0C (YBR_DIG_BASE + 0x30U)
    #define AS_MUTE_EN (u32)(0x1U << 31)//[31:31]
    #define AS_MUTE_HACT_EN (u32)(0x1U << 30)//[30:30]
    #define AS_MUTE_VACT_EN (u32)(0x1U << 29)//[29:29]
    #define AS_MUTE_CACT_EN (u32)(0x1U << 28)//[28:28]
    #define AS_MUTE_HP_EN (u32)(0x1U << 27)//[27:27]
    #define AS_MUTE_VP_EN (u32)(0x1U << 26)//[26:26]
    #define AS_MUTE_HLEN_EN (u32)(0x1U << 25)//[25:25]
    #define AS_MUTE_VLEN_EN (u32)(0x1U << 24)//[24:24]
    #define AS_MUTE_MULTI_EN (u32)(0x7FU << 24) //[30:24]
    #define AS_MUTE_H_CNT_THR (u32)(0xFU << 20)//[23:20]
    #define AS_H_DIFF_TH (u32)(0xFFU << 12)//[19:12]
    #define AS_H_STABLE_VALUE (u32)(0xFFFU)//[11:0]
#define ASYNC_0D (YBR_DIG_BASE + 0x34U)
    #define AS_MUTE_CLR (u32)(0x1U << 31)//[31:31]
    #define AS_C_SYNC_H_MUTE (u32)(0x3U << 29)//[30:29]
    #define AS_MUTE_V_CNT_THR (u32)(0xFU << 20)//[23:20]
    #define AS_V_DIFF_TH (u32)(0xFFU << 12)//[19:12]
    #define AS_V_STABLE_VALUE (u32)(0xFFFU)//[11:0]
#define ASYNC_0E (YBR_DIG_BASE + 0x38U)
    #define AS_DISABLE_DATA_ACTIVE (u32)(0x1U << 31)//[31:31]
    #define AS_HSYNC_LOCK_INV (u32)(0x1U << 30)//[30:30]
    #define AS_POST_DATA_ACTIVE_12 (u32)(0x1U << 29)//[29:29]
    #define AS_PRE_DATA_ACTIVE_12 (u32)(0x1U << 28)//[28:28]
    #define AS_SP0_PIXEL_EN (u32)(0x1U << 27)//[27:27]
    #define AS_C_MAXMIN_FILTER (u32)(0x3U << 25)//[26:25]
    #define AS_PIX_VCNT_FLD_RST_EN (u32)(0x1U << 24)//[24:24]
    #define AS_POST_DATA_ACTIVE_11_0 (u32)(0xFFFU << 12)//[23:12]
    #define AS_PRE_DATA_ACTIVE_11_0 (u32)(0xFFFU)//[11:0]
#define ASYNC_0F (YBR_DIG_BASE + 0x3CU)
    #define AS_TOP_THR (u32)(0xFFU << 24)//[31:24]
    #define AS_OVER_SAMPLE_PHASE (u32)(0x1U << 23)//[23:23]
    #define AS_OVER_SAMPLE_ON (u32)(0x1U << 22)//[22:22]
    #define AS_PHASESEL_B_DISABLE (u32)(0x1U << 21)//[21:21]
    #define AS_PHASESEL_GX (u32)(0x1FU << 16)//[20:16]
    #define AS_PHASE_SUM_ALG_SEL (u32)(0x1U << 15)//[15:15]
    #define AS_PHASE_RESET (u32)(0x1U << 14)//[14:14]
    #define AS_PHASESEL_G_DISABLE (u32)(0x1U << 13)//[13:13]
    #define AS_PHASESEL_BX (u32)(0x1FU << 8)//[12:8]
    #define AS_PHASE_MAXMIN_SEL (u32)(0x1U << 7)//[7:7]
      #define AS_PHASE_MAX_SEL (u32)0U
      #define AS_PHASE_MIN_SEL (u32)1U
    #define AS_PHASE_RAND_EN (u32)(0x1U << 6)//[6:6]
    #define AS_PHASESEL_R_DISABLE (u32)(0x1U << 5)//[5:5]
    #define AS_PHASESEL_RX (u32)(0x1FU)//[4:0]
#define ASYNC_10 (YBR_DIG_BASE + 0x40U)
    #define AS_HLOCK_SELF_END_12 (u32)(0x1U << 31)//[31:31]
    #define AS_HLOCK_SELF_ST_12 (u32)(0x1U << 30)//[30:30]
    #define AS_H_LEN_PIX_STABLE_TH (u32)(0x3FU << 24)//[29:24]
    #define AS_HLOCK_SELF_END_11_0 (u32)(0xFFFU << 12)//[23:12]
    #define AS_HLOCK_SELF_ST_11_0 (u32)(0xFFFU)//[11:0]
#define ASYNC_11 (YBR_DIG_BASE + 0x44U)
    #define AS_HWIDTH_MEASURE (u32)(0x1U << 31)//[31:31]
    #define AS_H_STABLE_MON_EN (u32)(0x1U << 30)//[30:30]
    #define AS_VSYNC_OUT_DELAY_SEL (u32)(0x1U << 29)//[29:29]
    #define AS_VSYNC_OUTP_INV (u32)(0x1U << 28)//[28:28]
    #define AS_NEW_VS_OUTP_H1 (u32)(0xFFFU << 12)//[23:12]
    #define AS_NEW_VS_OUTP_S1 (u32)(0xFFFU)//[11:0]
#define ASYNC_12 (YBR_DIG_BASE + 0x48U)
    #define AS_VCNT_PIX_RESET_FIELD (u32)(0x1U << 31)//[31:31]
    #define AS_FLD_SELECT (u32)(0x3U << 29)//[30:29]
        #define FIELD_DET_CLK27 (u32)0
        #define FIELD_FREERUN   (u32)1
        #define FIELD_DISABLE   (u32)3
    #define AS_VS_OUT_INT_SEL (u32)(0x1U << 28)//[28:28]
        #define VS_PROG_VSYNC_INT   (u32)0
        #define VS_INPUT_VSYNC_INT  (u32)1
    #define AS_DISABLE_VS_OUT (u32)(0x1U << 27)//[27:27]
    #define AS_VS_UP_GATE (u32)(0x1U << 25)//[25:25]
    #define AS_VS_UPDATE_EN (u32)(0x1U << 24)//[24:24]
    #define AS_NEW_VS_OUTP_H2 (u32)(0xFFFU << 12)//[23:12]
    #define AS_NEW_VS_OUTP_S2 (u32)(0xFFFU)//[11:0]
#define ASYNC_13 (YBR_DIG_BASE + 0x4CU)
    #define AS_BDMASK_DISABLE (u32)(0x1U << 31)//[31:31]
    #define AS_BDVSYNCP (u32)(0x1U << 28)//[28:28]
    #define AS_BDHSYNCP (u32)(0x1U << 27)//[27:27]
    #define AS_BDINSEL (u32)(0x7U << 24)//[26:24]
    #define AS_BDMASK_END (u32)(0xFFFU << 12)//[23:12]
    #define AS_BDMASK_ST (u32)(0xFFFU)//[11:0]
#define ASYNC_14 (YBR_DIG_BASE + 0x50U)
    #define AS_VFREE_RST_EN (u32)(0x1U << 31)//[31:31]
    #define AS_HFREE_RST_EN (u32)(0x1U << 30)//[30:30]
    #define AS_HFREE_RST_TH_12 (u32)(0x1U << 24)//[24:24]
    #define AS_VFREE_RST_TH (u32)(0xFFFU << 12)//[23:12]
    #define AS_HFREE_RST_TH_11_0 (u32)(0xFFFU)//[11:0]
#define ASYNC_15 (YBR_DIG_BASE + 0x54U)
    #define AS_BDDATATH (u32)(0xFFU << 24)//[31:24]
    #define AS_H_BD_MASK_R_11_0 (u32)(0xFFFU << 12)//[23:12]
    #define AS_H_BD_MASK_L_11_0 (u32)(0xFFFU)//[11:0]
#define ASYNC_16 (YBR_DIG_BASE + 0x58U)
    #define AS_H_BD_MASK_R_12 (u32)(0x1U << 31)//[31:31]
    #define AS_AUTO_CLK_RDY_CLR (u32)(0x1U << 30)//[30:30]
    #define AS_CLK_AUTO (u32)(0x1U << 29)//[29:29]
    #define AS_CLKDET_INI (u32)(0x1U << 28)//[28:28]
    #define AS_H_BD_MASK_L_12 (u32)(0x1U << 27)//[27:27]
    #define AS_AUTO_PHASE_RDY_CLR (u32)(0x1U << 26)//[26:26]
    #define AS_PHASE_AUTO (u32)(0x1U << 25)//[25:25]
    #define AS_PHSDET_INI (u32)(0x1U << 24)//[24:24]
    #define AS_PSNE_THB1 (u32)(0xFFU << 16)//[23:16]
    #define AS_PSNE_THG1 (u32)(0xFFU << 8)//[15:8]
    #define AS_PSNE_THR1 (u32)(0xFFU)//[7:0]
#define ASYNC_17 (YBR_DIG_BASE + 0x5CU)
    #define AS_TOP_PE_SW (u32)(0x1U << 31)//[31:31]
    #define AS_C_PSNE_SRC_SEL (u32)(0x3U << 29)//[30:29]
    #define AS_C_PSNE_STA_SEL (u32)(0x1U << 28)//[28:28]
    #define AS_PHASE_BOUNDARY_ENABLE (u32)(0x1 << 27)//[27:27]
    #define AS_PSNE_THB2 (u32)(0xFFU << 16)//[23:16]
    #define AS_PSNE_THG2 (u32)(0xFFU << 8)//[15:8]
    #define AS_PSNE_THR2 (u32)(0xFFU)//[7:0]
#define ASYNC_18 (YBR_DIG_BASE + 0x60U)
    #define AS_VMASK3_ST (u32)(0xFFU << 24)//[31:24]
    #define AS_VMASK3_END (u32)(0xFFU << 16)//[23:16]
    #define AS_MAX_RST_CNT_THR (u32)(0x1FU << 8)//[12:8]
    #define AS_TOTAL_LINE_THR (u32)(0xFFU)//[7:0]
#define ASYNC_19 (YBR_DIG_BASE + 0x64U)
    #define AS_SDDS_INIT_NEW_EN (u32)(0x1U << 31)//[31:31] ==> yunjie mark
    #define AS_INIT_WINDOW_THR (u32)(0xFFFU << 19)//[30:19] ==> yunjie mark
    #define AS_VMASK4_MODE (u32)(0x1U << 18)//[18:18]
    #define AS_VMASK4_OFF (u32)(0x1U << 17)//[17:17]
    #define AS_VMASK4_INV (u32)(0x1U << 16)//[16:16]
    #define AS_VMASK4_END (u32)(0xFFU << 8)//[15:8]
    #define AS_VMASK4_ST (0xFFU)//[7:0]
#define ASYNC_1A (YBR_DIG_BASE + 0x68U)
    #define AS_PIX_AUTO_HV_SKEW (u32)(0x1U << 28)//[28:28]
    #define AS_AUTO_HV_SKEW_PIX_EN (u32)(0x1U << 27)//[27:27]
    #define AS_VIO_CLR (u32)(0x1U << 26)//[26:26]
    #define AS_VIOLATION_DET_EN (u32)(0x1U << 25)//[25:25]
    #define AS_STBL_WIN_MON_EN (u32)(0x1U << 24)//[24:24]
    #define AS_STBL_WIN_END (u32)(0xFFFU << 12)//[23:12]
    #define AS_STBL_WIN_ST (u32)(0xFFFU)//[11:0]
 /**
MISC Register
*/
#define MISC (YBR_DIG_BASE + 0x6CU)
    #define C_FT_MODE (u32)(0x1U << 31) //[31:31]
    #define C_FT_COLOR_BAR (u32)(0x1U << 30) //[30:30]
    #define C_VIDEO_FORMAT (u32)(0x1U << 29) //[29:29]
    #define C_FT_SPEED_UP (u32)(0x1U << 28) //[28:28]
    #define YPBPR_DBG_DISBALE (u32)(0x1U << 11)//[11:11]
    #define DBG_SEL (u32)(0x3U << 8U) //[9:8]
    #define HDTV_DBG_MODE (u32)(0xFU << 4) //[7:4]
    #define SDDS_INIT_REG (u32)(0x1U << 2) //[2:2]
    #define C_RETIME_AUTO (u32)(0x1U << 1) //[1:1]
    #define C_MISC_CG_DIS (u32)(0x1U) //[0:0]     
/**
*/
#define VGA_ANAIF_CTRL (YBR_DIG_BASE + 0x84U)
    #define VGA_MON_EN (u32)(0x1U << 18) //[18:18]
    #define VGA_MON_SEL (u32)(0x3U << 16) //[17:16]
    #define MON_FIFO_START (u32)(0x1U << 10) //[10:10]
    #define PHASE_SEL (u32)(0x7U << 7U) // [9:7]
    #define DOWN_SAMPL_MODE (u32)(0x7U << 4) //[6:4]
    #define DOWN_SAMPL_ENABLE (u32)(0x1U << 3) //[3:3]
    #define MON_SEL (u32)(0x3U << 1) //[2:1]
    
/**
HDTV Register
*/
#define HDFE_00 (YBR_DIG_BASE + 0x88U)
    #define AD3_OFFSET (u32)(0x1FFU << 20) //[28:20]
    #define AD2_OFFSET (u32)(0x1FFU << 10) //[18:10]
    #define AD1_OFFSET (u32)(0x1FFU) //[8:0]
#define HDFE_01 (YBR_DIG_BASE + 0x8CU)
    #define AD1_GAIN (u32)(0xFFFFU << 16) //[31:16]
    #define AD2_GAIN (u32)(0xFFFFU) //[15:0]
#define HDFE_02 (YBR_DIG_BASE + 0x90U)
    #define AD3_GAIN (u32)(0xFFFFU << 16) //[31:16]
    #define ADC_FT_MODE (0x1U << 4) //[4:4]
    #define IDX_CHANNEL_EN (u32)(0xFU) //[3:0]
#define HDFE_03 (YBR_DIG_BASE + 0x94U)
    #define AD3_GAIN_BIAS (u32)(0x3FFU << 20) //[29:20]
    #define AD2_GAIN_BIAS (u32)(0x3FFU << 10) //[19:10]
    #define AD1_GAIN_BIAS (u32)(0x3FFU) //[9:0]
#define HDFE_04 (YBR_DIG_BASE + 0x98U)
    #define AD1_GAIN_Temp (u32)(0xFFFU << 16) //[27:16]
    #define AD2_GAIN_Temp (u32)(0xFFFU) //[11:0]
#define HDFE_05 (YBR_DIG_BASE + 0x9CU)
    #define AD3_GAIN_Temp (u32)(0xFFFU) //[11:0]

#define HDTV_00 (YBR_DIG_BASE + 0xA0U)
    #define HDTV_ADC_LSB_CLR (u32)(0x1U << 31)//[31:31]
    #define HDTV_ADC3_CLR (u32)(0x1U << 30)//[30:30]
    #define HDTV_ADC2_CLR (u32)(0x1U << 29)//[29:29]
    #define HDTV_ADC1_CLR (u32)(0x1U << 28)//[28:28]
    #define ADC_FROM_DVI (u32)(0x1U << 27)//[27:27]
    #define HDTV_ADC3_MID (u32)(0x1U << 26)//[26:26]
    #define HDTV_ADC2_MID (u32)(0x1U << 25)//[25:25]
    #define HDTV_ADC1_MID (u32)(0x1U << 24)//[24:24]
    #define HDTV_SP0_F (u32)(0x1U << 23U)//[23:23]
    #define HDTV_BLANK_ON (u32)(0x1U << 22)//[22:22]
    #define ADC1_DELAY_1T (u32)(0x1U << 21)//[21:21]
    #define ADC2_DELAY_1T (u32)(0x1U << 20)//[20:20]
    #define ADC3_DELAY_1T (u32)(0x1U << 19)//[19:19]
    #define HDTV_FIELD_ALIGN (u32)(0x1U << 18)//[18:18]
    #define HDTV_AVMASK_HEDGE (u32)(0x1U << 17)//[17:17]
    #define HDTV_AVMASK_SEL (u32)(0x1U << 16)//[16:16]
    #define HDTV_AVMASK_EN (u32)(0x1U << 15)//[15:15]
    #define HDTV_FLD_SEL (u32)(0x1U << 14)//[14:14]
    #define HDTV_RGB (u32)(0x1U << 13)//[13:13]
    #define HDTV_CEN_SEL (u32)(0x1U << 12)//[12:12]
    #define HDTV_LCLAMP_EN (u32)(0x1U << 11)//[11:11]
    #define HDTV_LCLAMP_ZERO (u32)(0x1U << 10)//[10:10]
    #define HDTV_BLK_CALI_MIN (u32)(0x7U << 7)//[9:7]
    #define HDTV_RAW_DATA_OUT (u32)(0x1U << 6)//[6:6]
    #define HDTV_BLK_STA_SEL (u32)(0x1U << 5)//[5:5]
        #define HDTV_BLK_STA_LINE_AVERAGE (u32)1
        #define HDTV_BLK_STA_CALIBRATION  0
    #define HDTV_BLK_CALI_PERIOD (u32)(0x7U << 2)//[4:2]
    #define HDTV_BLK_CALI_LCNT (u32)(0x3U)//[1:0]
#define HDTV_01 (YBR_DIG_BASE + 0xA4U)
    //#define HDTV_BLK_CALI_START (u32)(0x3FU << 25)//[31:25]
    #define HDTV_BLK_CALI_START (u32)(0x7FU << 25)//[31:25] 68031 modify it
    #define HDTV_AV_START (u32)(0x1FFFU << 12)//[24:12]
    #define HDTV_AV_WIDTH (u32)(0xFFFU)//[11:0]
#define HDTV_02 (YBR_DIG_BASE + 0xA8U)
    #define HDTV_BLK_CALI_ADJ_ON (u32)(0x1U << 31)//[31:31]
    //#define HDTV_BLK_CALI_Y_TAR (u32)(0x3FU << 24)//[30:24]
    #define HDTV_BLK_CALI_Y_TAR (u32)(0x7FU << 24)//[30:24] 68031 modify it
    #define HDTV_BLK_CALI_EN (u32)(0x1U << 23U)//[23:23]
    //#define HDTV_BLK_CALI_PB_TAR (u32)(0x3FU << 16)//[22:16]
    #define HDTV_BLK_CALI_PB_TAR (u32)(0x7FU << 16)//[22:16] 68031 modify it
    #define HDTV_BLK_CALI_FREEZE (u32)(0x1U << 15)//[15:15]
    //#define HDTV_BLK_CALI_PR_TAR (u32)(0x3FU << 8)//[14:8]
    #define HDTV_BLK_CALI_PR_TAR (u32)(0x7FU << 8)//[14:8] 68031 modify it
    #define HDTV_BLK_CALI_FCNT (u32)(0x3U << 6)//[7:6]
    #define HDTV_BLK_CALI_THRES (u32)(0x3FU)//[5:0]
#define HDTV_03 (YBR_DIG_BASE + 0xACU)
    #define HDTV_V_RELATCH_SEL (u32)(0x1U << 31)//[31:31]
    #define HDTV_V_RELATCH_POS (u32)(0x1U << 30)//[30:30]
    #define HDTV_HLOCK_NEG_LATCH (u32)(0x1U << 29)//[29:29]
    #define HDTV_EN (u32)(0x1U << 28)//[28:28]
    #define ADC_LSB_EN (u32)(0xFU << 24)//[27:24]
    #define HDTV_BLANK_AVG (u32)(0x3U << 22)//[23:22]
    #define HDTV_RAW_VSYNC_SEL1 (u32)(0x1U << 21)//[21:21]
    #define HDTV_RAW_VSYNC_SEL0 (u32)(0x1U << 20)//[20:20]
    #define HDTV_HEDGE_SEL (u32)(0x1U << 19)//[19:19]
        #define HDTV_RISING_EDGE    0
        #define HDTV_FALLING_EDGE   (u32)1
    #define HDTV_DATA_SEL (u32)(0x7U << 16)//[18:16]
    #define HDTV_H_SEL (u32)(0x3U << 14) //[15:14] 
        #define SP0_HS_OUT (u32)1
        #define HSYNC_LOCK_SEL 0//select by setting HDTV_HLOCK_NEG_LATCH
    #define HDTV_V_SEL (u32)(0x3U << 12) //13:12
    #define HDTV_BLANK_CLEAR (u32)(0x1U << 11)//[11:11]
    #define HDTV_BLANK_HOLD (u32)(0x1U << 10)//[10:10]
    #define HDTV_BLAK_SET (u32)(0x3U << 10) //[11:10]
        #define AS_BLANK_RESET (u32)0x02
        #define AS_BLANK_HOLD (u32)0x01
        #define AS_BLANK_ALWAYS 0x00
    #define HDTV_BLANK_START (u32)(0x3FFU)//[9:0]

#define HDTV_04 (YBR_DIG_BASE + 0xB0U)
    #define HDTV_CLAMP_START (u32)(0xFFFU << 12)//[23:12]
    #define HDTV_CLAMP_END (u32)(0xFFFU)//[11:0]
#define HDTV_05 (YBR_DIG_BASE + 0xB4U)
    #define HDTV_YOFFSET (u32)(0x3FU << 26)//[31:26]
    #define HDTV_BLANK_EF_EN (u32)(0x1U)//[0:0]
#define HDTV_06 (YBR_DIG_BASE + 0xB8)
    #define CLAMP_HD_en (u32)(0x1U << 12) //[12:12]
    #define CLAMP_HD_THRE (u32)(0xFFFU) //[11:0]

/**
    INT 
*/
#define INT_YBR_VGA_MASK (YBR_DIG_BASE + 0xBCU)
#define INT_YBR_VGA_STA   (YBR_DIG_BASE + 0x3FCU)
    #define INT_ALL (u32)(0xF)
    #define INT_MUTE (u32)(0x1)
    #define INT_VSYNC (u32)(0x1U << 1)
    #define INT_MODE_CHANGE (u32)(0x1U << 2)
    #define INT_DDS_LOCK (u32)(0x1U << 3)

#define  fgIsVdoIntSp0Vsyncout() (g_u4IrqStatus & INT_VSYNC)
#define fgIsVdoIntSp0Mute() (g_u4IrqStatus & INT_MUTE)

/**
    SYNC2 Status
*/
#define STA_SYNC2_00 (YBR_DIG_BASE + 0xC0U)
    #define AS2_CSYNC_ACT (u32)(0x1U << 24)//[24:24]
    #define AS2_VSYNC_WIDTH_S (u32)(0xFFFU << 12)//[23:12]
    #define AS2_HSYNC_WIDTH_S (u32)(0xFFFU)//[11:0]
#define STA_SYNC2_01 (YBR_DIG_BASE + 0xC4)
    #define AS2_H_LEN_S (u32)(0xFFFU << 12)//[23:12]
    #define AS2_V_LEN_S (u32)(0xFFFU)//[11:0]


/**
    HDTV Status
*/
#define HDTV_STA_00 (YBR_DIG_BASE + 0xC8U)
    #define STA_HDTV_BLANK_Y (u32)(0x3FFU << 20)//[29:20]
    #define STA_HDTV_BLANK_PB (u32)(0x3FFU << 10)//[19:10]
    #define STA_HDTV_BLANK_PR (u32)(0x3FFU)//[9:0]

 /**
    SYNC0 Status
*/
#define STA_SYNC0_00 (YBR_DIG_BASE + 0xCCU)
    #define AS_H_LEN_S (u32)(0xFFFU << 20)//[31:20]
    #define AS_V_LEN_S (u32)(0xFFFU << 8)//[19:8]
    #define AS_MODE_CHG_STA (0x1U << 7)//[7:7]
    #define AS_HSYNC_P (u32)(0x1U << 6)//[6:6]
    #define AS_VSYNC_P (u32)(0x1U << 5)//[5:5]
    #define AS_FIELD_CK27_DET (u32)(0x1U << 4)//[4:4]
    #define AS_FIELD_ACT (u32)(0x1U << 3)//[3:3]
    #define AS_HSYNC_ACT (u32)(0x1U << 2)//[2:2]
    #define AS_VSYNC_ACT (u32)(0x1U << 1)//[1:1]
    #define AS_CSYNC_ACT (u32)(0x1)//[0:0]
    #define AS_SYNC_ACT (u32)(0x7U)
      #define STA0_HSYNC_ACT 0x04  // detect Hsync Activity , 1 : active
      #define STA0_VSYNC_ACT 0x02  // detect Vsync Activity , 1 : active
      #define STA0_CSYNC_ACT 0x01  // detect Csync Activity , 1 : active  
#define STA_SYNC0_01 (YBR_DIG_BASE + 0xD0U)
    #define AS_SYNC0_MUTE (u32)(0x1U << 31)//[31:31]
    #define AS_AUTO_DELAY_TOG (u32)(0x1U << 30)//[30:30]
    #define AS_AUTO_DELAY_VSYNC (u32)(0x1U << 29)//[29:29]
    #define AS_HSYNC_WIDTH_S (u32)(0xFFFU << 12)//[23:12]
    #define AS_VSYNC_WIDTH_S (u32)(0xFFFU)//[11:0]
#define STA_SYNC0_02 (YBR_DIG_BASE + 0xD4U)
    #define AS_SOGH_STABLE (u32)(0x1U << 31)//[31:31]
    #define AS_SOGV_STABLE (u32)(0x1U << 30)//[30:30]
    #define AS_VSYNC_OUTX_ACT (u32)(0x1U << 24)//[24:24]
    #define AS_CH_LEN_S (u32)(0xFFFU << 12)//[23:12]
    #define AS_CV_LEN_S (u32)(0xFFFU)//[11:0]
#define STA_SYNC0_03 (YBR_DIG_BASE + 0xD8U)
    #define AS_V_LEN0_CK27_WIDTH (u32)(0x1FFFU << 16)//[28:16]
    #define AS_F1_PSYNC_NO (u32)(0xFFU << 8)//[15:8]
    #define AS_F0_PSYNC_NO (u32)(0xFFU)//[7:0]
#define STA_SYNC0_04 (YBR_DIG_BASE + 0xDCU)
    #define AS_H_LEN_PIX_S_7_0 (u32)(0xFFU << 24)//[31:24]
    #define AS_LEFTBC_STA_S_11_0 (u32)(0xFFFU << 12)//[23:12]
    #define AS_RIGHTBC_STA_S_11_0 (0xFFFU)//[11:0]
#define STA_SYNC0_05 (YBR_DIG_BASE + 0xE0U)
    #define AS_HSYNC_WIDTH_PIX_S_7_0 (u32)(0xFFU << 24)//[31:24]
    #define AS_TOPBC_STA_S (u32)(0xFFFU << 12U)//[23:12]
    #define AS_BOTTOMBC_STA_S (u32)(0xFFFU)//[11:0]
#define STA_SYNC0_06 (YBR_DIG_BASE + 0xE4U)
    #define AS_HSYNC_WIDTH_PIX_S_11_8 (u32)(0xFU << 28)//[31:28]
    #define AS_H_LEN_PIX_S_11_8 (u32)(0xFU << 24)//[27:24]
    #define AS_NEWTOPBC_S (u32)(0xFFFU << 12)//[23:12]
    #define AS_NEWBOTTOMBC_S (u32)(0xFFFU)//[11:0]
#define STA_SYNC0_07 (YBR_DIG_BASE + 0xE8U)
    #define AS_RMAXMIND (u32)(0xFFU << 24)//[31:24] //R:LPF out
    #define AS_TOP_SUMRD_S (u32)(0xFFFFFFU)//[23:0]
#define STA_SYNC0_08 (YBR_DIG_BASE + 0xECU)
    #define AS_GMAXMIND (u32)(0xFFU << 24)//[31:24] //G: LPF out
    #define AS_TOP_SUMGD_S (u32)(0xFFFFFFU)//[23:0]
#define STA_SYNC0_09 (YBR_DIG_BASE + 0xF0U)
    #define AS_BMAXMIND (u32)(0xFFU << 24)//[31:24] //B:LPF out
    #define AS_TOP_SUMBD_S (u32)(0xFFFFFFU)//[23:0]
#define STA_SYNC0_0A (YBR_DIG_BASE + 0xF4U)
    #define AS_H_LEN_PIX_S_12 (u32)(0x1U << 31)//[31:31]
    #define AS_HSYNC_WIDTH_PIX_S_12 (u32)(0x1U << 30)//[30:30]
    #define AS_LEFTBC_STA_S_12 (u32)(0x1U << 29)//[29:29]
    #define AS_RIGHTBC_STA_S_12 (u32)(0x1U << 28)//[28:28]
    #define AS_STA_R_S (u32)(0xFFFFFFU)//[23:0]
#define STA_SYNC0_0B (YBR_DIG_BASE + 0xF8U)
    #define AS_HV_HIT_PIX (u32)(0x1U << 31)//[31:31]
    #define AS_R_DLY (u32)(0x1U << 30)//[30:30]
    #define AS_G_DLY (u32)(0x1U << 29)//[29:29]
    #define AS_B_DLY (u32)(0x1U << 28)//[28:28]
    #define AS_STA_G_S (u32)(0xFFFFFFU)//[23:0]
#define STA_SYNC0_0C (YBR_DIG_BASE + 0xFCU)
    #define AS_AUTO_PHASE_RDY (u32)(0x1U << 29)//[29:29]
    #define AS_PHASE_GOOD (u32)(0x1FU << 24)//[28:24]
    #define AS_STA_B_S (u32)(0xFFFFFFU)//[23:0]
#define STA_SYNC0_0D (YBR_DIG_BASE + 0x100U)
    //#define AS_V_LEN0_WIDTH (u32)(0x1FFFFU)//[12:0]
    #define AS_V_LEN0_WIDTH (u32)(0x1FFFU)//[12:0] 68031 modify it
#define STA_SYNC0_0E (YBR_DIG_BASE + 0x104U)
    #define AS_R_VIOLATION (u32)(0x1U << 31)//[31:31]
    #define AS_G_VIOLATION (u32)(0x1U << 30)//[30:30]
    #define AS_B_VIOLATION (u32)(0x1U << 29)//[29:29]
    #define AS_WINB_HSTABLE (u32)(0x1U << 28)//[28:28]
    #define AS_H_LEN_PIX_STABLE (u32)(0x1U << 27)//[27:27]
    #define AS_AUTO_CLK_RDY (u32)(0x1U << 26)//[26:26]
    #define AS_PHS_MAXMIN_DIFF_S (u32)(0x3FFFFFFU)//[25:0]
/*
    Color Trans
*/
 
#define COLORTRANS0 (YBR_DIG_BASE + 0x180U)
    #define EXT_EN  (u32)(0x1U << 21) //[21:21]
    #define IN_TAB_SEL (u32)(0x7U << 18) //[20:18]
    #define CT_BYPASS (u32)(0x1U << 17) //[17:17]
    #define CT_EN (u32)(0x1U << 16) //[16:16]
    #define CH1_GAIN (u32)(0x1FFFU) //[12:0]
#define COLORTRANS1 (YBR_DIG_BASE + 0x184U)
    #define CH3_GAIN (u32)(0x1FFFU << 13) //[25:13]
    #define CH2_GAIN (u32)(0x1FFFU) //[12:0]
#define COLORTRANS2 (YBR_DIG_BASE + 0x188U)
    #define CH3_PRE_ADDR (u32)(0x1FFU << 20) //[28:20]
    #define CH2_PRE_ADDR (u32)(0x1FFU << 10) //[18:10]
    #define CH1_PRE_ADDR (u32)(0x1FFU) //[8:0]
#define COLORTRANS3 (YBR_DIG_BASE + 0x18CU)
    #define CH3_POST_ADDR (u32)(0x1FFU << 20) //[28:20]
    #define CH2_POST_ADDR (u32)(0x1FFU << 10) //[18:10]
    #define CH1_POST_ADDR (u32)(0x1FFU) //[8:0]

/**
VFE Register(SDDS_Register) 
*/
#define VFE_14 (YBR_DIG_BASE + 0x70U)
    #define DCLK_PFD_SEL (u32)(0xFU << 28) //[31:28]
    #define LOCK_THR (u32)(0x3U << 26) //[27:26]
    #define LOCK_CNT (u32)(0x3U << 24) //[25:24]
    #define ERR_LIM (u32)(0xFFU << 16) //[23:16]
    #define MAX_PERIOD (u32)(0x3U << 14) //[15:14]
    #define FM_PERIOD (u32)(0x3U << 12) //[13:12]
    #define DCLK1_KI_1 (u32)(0x3U << 10) //[11:10]
    #define DCLK1_KP_1 (u32)(0x3U << 8) //[9:8]
    #define DCLK1_KI_0 (u32)(0xFU << 4) //[7:4]
    #define DCLK1_KP_0 (u32)(0xFU) //[3:0]
#define VFE_15 (YBR_DIG_BASE + 0x74U)
    #define DCLK_FAST_LF (u32)(0xFU << 28) //[31:28]
    #define DCLK_FREQ_CW (u32)(0xFFFFFFFU) //[27:0]
#define VFE_16 (YBR_DIG_BASE + 0x78U)
    #define DCLK_HSYNC_WIDTH (u32)(0x3FFFU << 16) //[29:16]
    #define DCLK_HLINE_LENGTH (u32)(0x3FFFU) //[13:0]
#define VFE_17 (YBR_DIG_BASE + 0x7CU)
    #define DIFF_THR (u32)(0xFFU << 24) //[31:24]
    #define SDDS_CKSEL (u32)(0xFFU << 16) //[23:16]
    #define DCLK_RESETB (u32)(0x1U << 15) //[15:15]
    #define DCLK_INIT (u32)(0x1U << 14) //[14:14]
    #define HSYNC_POL_VFE (u32)(0x1U << 13) //[13:13]
    #define COMP_EN (u32)(0x1U << 12) //[12:12]
    #define DCLK2_KI_1 (u32)(0x3U << 10) //[11:10]
    #define DCLK2_KP_1 (u32)(0x3U << 8) //[9:8]
    #define DCLK2_KI_0 (u32)(0xFU << 4) //[7:4]
    #define DCLK2_KP_0 (u32)(0xFU) //[3:0]
#define VFE_18 (YBR_DIG_BASE + 0x80U)
    #define RG_SDDS_DATA_SYNC (u32)(0x1U << 20) //[20:20]
    #define RG_SDDS_FIFO_START (u32)(0x1U << 19) //[19:19]
    #define SDDS_CLK3_INV (u32)(0x1U << 18) //[18:18]
    #define RG_SDDS_FRAC_SEL_INV (u32)(0x1U << 17) //[17:17]
    #define SDDS_CLK1_INV (u32)(0x1U << 16) //[16:16]
    #define JTR_SEL (u32)(0x1U << 15) //[15:15]
    #define DLY_SEL (u32)(0x1U << 14) //[14:14]
    #define DDS_HLINE (0x1FFFU) //[12:0]
#define VFE_STA_00 (YBR_DIG_BASE + 0x108U)
    #define DDS_LOCK (u32)(0x1U << 25) //[25:25]
    #define DDS_FM_JITTER (u32)(0x1U << 24) //[24:24]
    #define DDS_LPERR (u32)(0xFFFU << 12) //[23:12] 
    #define DDS_LOCK_LINE  (0xFFFU) //[11:0] 
#define VFE_STA_01 (YBR_DIG_BASE + 0x10CU)
    #define DDS_DIVISOR (u32)(0xFU << 24) //[27:24]
    #define DDS_FREQ_CW (u32)(0xFFFFFFU) //[23:0]
#define VFE_STA_02 (YBR_DIG_BASE + 0x110U)
    //#define DDS_MAX_PERR (u32)(0x3FFU << 8) //[18:8]
    #define DDS_MAX_PERR (u32)(0x7FFU << 8) //[18:8] 68031 modify it
    #define DDS_PHASE_TSEL (u32)(0xFFU) //[7:0]

/**
REG_PLLGP_CFG
*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define IO_PLLGP_BASE (IO_BASE_VA + 0x00000000U)
#define REG_PLL_GROUP_CFG18 (IO_PLLGP_BASE + 0x5C8U)
    #define FLD_RG_TL_27M_SEL (u32)(0x1U << 23)//[23:23]   
#define REG_PLL_GROUP_CFG28 (IO_PLLGP_BASE + 0x5f0U)
    #define FLD_RG_VGAPLL_VODEN (u32)(0x1U << 30)//[30:30]
    #define FLD_RG_VGAPLL_HF (u32)(0x1U << 29)//[29:29]
    #define FLD_RG_VGAPLL_LF (u32)(0x1U << 28)//[28:28]
    #define FLD_RG_VGAPLL_V11EN (u32)(0x1U << 27)//[27:27]
    #define FLD_RG_VGAPLL_BP (u32)(0x1U << 26)//[26:26]
    #define FLD_RG_VGAPLL_BR (u32)(0x1U << 25)//[25:25]
    #define FLD_RG_VGAPLL_FPEN (u32)(0x1U << 24)//[24:24]
    #define FLD_RG_VGAPLL_PREDIV (u32)(0x3U << 22)//[23:22]
    #define FLD_RG_VGAPLL_POSDIV (u32)(0x3U << 20)//[21:20]
    #define FLD_RG_VGAPLL_FBSEL (u32)(0x3U << 18)//[19:18]
    #define FLD_RG_VGAPLL_BS (u32)(0x3U << 16)//[17:16]
    #define FLD_RG_VGAPLL_DIVEN (u32)(0x7U << 13)//[15:13]
    #define FLD_RG_VGAPLL_MONEN (u32)(0x1U << 12)//[12:12]
    #define FLD_RG_VGAPLL_FMEN (u32)(0x1U << 11)//[11:11]
    #define FLD_RG_VGAPLL_DET_EN (u32)(0x1U << 10)//[10:10]
    #define FLD_RG_VGAPLL_RESERVE (u32)(0x3U << 8)//[9:8]
    #define FLD_RG_VGAPLL_MONCKEN (u32)(0x1U << 7)//[7:7]
    #define FLD_RG_VGAPLL_FBDIV (u32)(0x7FU)//[6:0]
#define REG_PLL_GROUP_CFG29 (IO_PLLGP_BASE + 0x5f4U)
    #define FLD_RG_VGAPLL_SDDS_PD_PDB (u32)(0x1U << 31)//[31:31]
    #define FLD_RG_VGAPLL_SDDS_PD_EN (u32)(0x1U << 30)//[30:30]
    #define FLD_RG_XDDS_CKSEL (u32)(0x3U << 26)//[27:26]
    #define FLD_RG_VGAPLL_CKO_SEL (u32)(0x3U << 24)//[25:24]
    #define FLD_RG_XDDS_PI_C (u32)(0x7U << 21)//[23:21]
    #define FLD_RG_XDDS_HF (u32)(0x1U << 20)//[20:20]
    #define FLD_RG_VGAPLL_R_EN (u32)(0x1U << 19)//[19:19]
    #define FLD_RG_VGAPLL_G_EN (u32)(0x1U << 18)//[18:18]
    #define FLD_RG_VGAPLL_B_EN (u32)(0x1U << 17)//[17:17]
    #define FLD_RG_VGAPLL_RTB_EN (u32)(0x1U << 16)//[16:16]
    #define FLD_RG_VGAPLL_ENTL (u32)(0xFU << 12)//[15:12]
    #define FLD_RG_VGAPLL_INTH_EN (u32)(0x1U << 11)//[11:11]
    #define FLD_RG_VGAPLL_EXTH_EN (u32)(0x1U << 10)//[10:10]
    #define FLD_RG_VGAPLL_SDDS_FBKSEL (u32)(0x3U << 8)//[9:8]
    #define FLD_RG_VGAPLL_SDDS_HSEL (u32)(0x3U << 6)//[7:6]
    #define FLD_RG_VGAPLL_SDDS_HPOR (u32)(0x1U << 5)//[5:5]
    #define FLD_RG_VGATL_RESERVE (u32)(0x7U << 2)//[4:2]
    #define FLD_RG_XDDS_MONEN (u32)(0x1U)//[0:0]
    #define VGAPLL_CKO_SEL (u32)(0x1U << 24)
    #define VGAPLL_RGB_EN  (u32)(0x7U << 17)
#define REG_PLL_GROUP_CFG30 (IO_PLLGP_BASE + 0x5f8U)// ==> REG_PLL_GROUP_CFG7
    #define FLD_RG_VGATL_TSELA (u32)(0xFU << 28)//[31:28]
    #define FLD_RG_VGATL_TSELB (u32)(0xFU << 24)//[27:24]
#define RGS_PLL_GROUP_CFG0 (IO_PLLGP_BASE + 0x5fcU)
    #define RGS_APLL_VCO_STATE (u32)(0x3FU << 26)//[31:26]
    #define RGS_APLL_VCOCAL_CPLT (u32)(0x1U << 25)//[25]
    #define RGS_APLL_VCOCAL_FAIL (u32)(0x1U << 24)//[24]
    #define RGS_APLL1_VCO_STATE  (u32)(0x3FU << 18)//[23:18]
    #define RGS_APLL1_VCOCAL_CPLT (u32)(0x1U << 17)//[17]
    #define RGS_APLL1_VCOCAL_FAIL (u32)(0x1U << 16)//[16]
#define REG_PLL_CTRL (IO_PLLGP_BASE + 0x24180U)
    #define FLD_RG_VGATL_BIAS_PWD (u32)(0x1U << 19)//[19:19] 
    #define FLD_RG_XDDS_PWDB (u32)(0x1U << 18)//[18:18] 
    #define FLD_RG_VGAPLL_PWD (u32)(0x1U << 17)//[17:17]
    #define FLD_RG_SYSPLL1_PWD (u32)(0x1U)//[0:0]

#else
#define IO_PLLGP_BASE (IO_BASE_REG_VA + 0x00000000U)
#define REG_PLL_GROUP_CFG0 (IO_PLLGP_BASE + 0x580U)
    //#define FLD_RG_SYSPLL1_PWD (u32)(0x1U << 31)//[31:31]
#define REG_PLL_GROUP_CFG18 (IO_PLLGP_BASE + 0x5C8U)
    #define FLD_RG_TL_27M_SEL (u32)(0x1U << 8)//[8:8]
#define REG_PLL_GROUP_CFG29 (IO_PLLGP_BASE + 0x5f4U)
    #define FLD_RG_VGAPLL_VODEN (u32)(0x1U << 14)//[14:14]
    #define FLD_RG_VGAPLL_HF (u32)(0x1U << 13)//[13:13]
    #define FLD_RG_VGAPLL_LF (u32)(0x1U << 12)//[12:12]
    #define FLD_RG_VGAPLL_V11EN (u32)(0x1U << 11)//[11:11]
    #define FLD_RG_VGAPLL_BP (u32)(0x1U << 10)//[10:10]
    #define FLD_RG_VGAPLL_BR (u32)(0x1U << 9)//[9:9]
    #define FLD_RG_VGAPLL_FPEN (u32)(0x1U << 8)//[8:8]
    #define FLD_RG_VGAPLL_PREDIV (u32)(0x3U << 6)//[7:6]
    #define FLD_RG_VGAPLL_POSDIV (u32)(0x3U << 4)//[5:4]
    #define FLD_RG_VGAPLL_FBSEL (u32)(0x3U << 2)//[3:2]
    #define FLD_RG_VGAPLL_BS (u32)(0x3U << 0)//[1:0]
#define REG_PLL_GROUP_CFG30 (IO_PLLGP_BASE + 0x5f8U)
    #define FLD_RG_VGAPLL_DIVEN (u32)(0x7U << 29)//[31:29]
    #define FLD_RG_VGAPLL_MONEN (u32)(0x1U << 28)//[28:28]
    #define FLD_RG_VGAPLL_FMEN (u32)(0x1U << 27)//[27:27]
    #define FLD_RG_VGAPLL_DET_EN (u32)(0x1U << 26)//[26:26]
    #define FLD_RG_VGAPLL_RESERVE (u32)(0x3U << 24)//[25:24]
    #define FLD_RG_VGAPLL_MONCKEN (u32)(0x1U << 23)//[23:23]
    #define FLD_RG_VGAPLL_FBDIV (u32)(0x7FU << 16)//[22:16]
    #define FLD_RG_VGAPLL_SDDS_PD_PDB (u32)(0x1U << 15)//[15:15]
    #define FLD_RG_VGAPLL_SDDS_PD_EN (u32)(0x1U << 14)//[14:14]
    #define FLD_RG_XDDS_CKSEL (u32)(0x3U << 10)//[11:10]
    #define FLD_RG_VGAPLL_CKO_SEL (u32)(0x3U << 8)//[9:8]
    #define FLD_RG_XDDS_PI_C (u32)(0x7U << 5)//[7:5]
    #define FLD_RG_XDDS_HF (u32)(0x1U << 4)//[4:4]
    #define FLD_RG_VGAPLL_R_EN (u32)(0x1U << 3)//[3:3]
    #define FLD_RG_VGAPLL_G_EN (u32)(0x1U << 2)//[2:2]
    #define FLD_RG_VGAPLL_B_EN (u32)(0x1U << 1)//[1:1]
    #define FLD_RG_VGAPLL_RTB_EN (u32)(0x1U << 0)//[0:0]
#define REG_PLL_GROUP_CFG31 (IO_PLLGP_BASE + 0x5fcU)
    #define FLD_RG_VGAPLL_ENTL (u32)(0xFU << 28)//[31:28]
    #define FLD_RG_VGAPLL_INTH_EN (u32)(0x1U << 27)//[27:27]
    #define FLD_RG_VGAPLL_EXTH_EN (u32)(0x1U << 26)//[26:26]
    #define FLD_RG_VGAPLL_SDDS_FBKSEL (u32)(0x3U << 24)//[25:24]
    #define FLD_RG_VGAPLL_SDDS_HSEL (u32)(0x3U << 22)//[23:22]
    #define FLD_RG_VGAPLL_SDDS_HPOR (u32)(0x1U << 21)//[21:21]
    #define FLD_RG_VGATL_RESERVE (u32)(0x7U << 18)//[20:18]
    #define FLD_RG_XDDS_MONEN (u32)(0x1U << 16)//[16:16]
        #define VGAPLL_CKO_SEL (u32)(0x1U << 24)
        #define VGAPLL_RGB_EN  (u32)(0x7U << 17)
    #define FLD_RG_VGATL_TSELA (u32)(0xFU << 12)//[15:12]
    #define FLD_RG_VGATL_TSELB (u32)(0xFU << 8)//[11:8]
#define RGS_PLL_GROUP_CFG0 (IO_PLLGP_BASE + 0x614U)
    #define RGS_APLL_VCO_STATE (u32)(0x3FU << 26)//[31:26]
    #define RGS_APLL_VCOCAL_CPLT (u32)(0x1U << 25)//[25]
    #define RGS_APLL_VCOCAL_FAIL (u32)(0x1U << 24)//[24]
    #define RGS_APLL1_VCO_STATE  (u32)(0x3FU << 18)//[23:18]
    #define RGS_APLL1_VCOCAL_CPLT (u32)(0x1U << 17)//[17]
    #define RGS_APLL1_VCOCAL_FAIL (u32)(0x1U << 16)//[16]
#define REG_PLL_CTRL (IO_PLLGP_BASE + 0x24180U)
    #define FLD_RG_VGATL_BIAS_PWD (u32)(0x1U << 19)//[19:19] 
    #define FLD_RG_XDDS_PWDB (u32)(0x1U << 18)//[18:18] 
    #define FLD_RG_VGAPLL_PWD (u32)(0x1U << 17)//[17:17]
    #define FLD_RG_SYSPLL1_PWD (u32)(0x1U << 0)//[0:0]
#endif

/**
YPbPr/VGA CLK CFG
*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define IO_TOP_MISC (IO_BASE_VA + 0x00000000U)
#else
#define IO_TOP_MISC (IO_BASE_REG_VA + 0x00000000U)
#endif
#define YBR_VGA_CLK_CFG (IO_TOP_MISC + 0x360U)
    #define RG_XDDS_CLK_TST (u32)(0x1U << 18) //[18:18]
    #define RG_RESYNC_CLK_TST (u32)(0x1U << 17) //[17:17]
    #define RG_HDTV_CLK_TST (u32)(0x1U << 16) //[16:16]
    #define RG_XDDS_CLK_INV (u32)(0x1U << 10) //[10:10]
    #define RG_RESYNC_CLK_INV (u32)(0x1U << 9) //[9:9]
    #define RG_HDTV_CLK_INV (u32)(0x1U << 8) //[8:8]
    #define RG_XDDS_CLK_ON (u32)(0x1U << 2) //[2:2]
    #define RG_RESYNC_CLK_ON (u32)(0x1U << 1) //[1:1]
    #define RG_HDTV_CLK_ON (u32)(0x1U) //[0:0]
/**
ANA8 VGA
*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define IO_ANA8_VGA (IO_BASE_VA + 0x00000000U)
#define REG_VGA_Normal_CFG0 (IO_ANA8_VGA+0x6E0U)
#else
#define IO_ANA8_VGA (IO_BASE_REG_VA + 0x00000000U)
#define REG_VGA_Normal_CFG0 (IO_ANA8_VGA + 0x744U)
#endif
    #define RG_SHORT_FEO (u32)(0x1U << 29) //[29:29]
    #define RG_ADCTEST_EN (u32)(0x1U << 28) //[28:28]
    #define RG_VREFGEN4FE_PWD (u32)(0x1U << 24) //[24:24]
    #define RG_VDC_P_EN (u32)(0x1U << 23) //[23:23]
    #define RG_VDC_N_EN (u32)(0x1U << 22) //[22:22]
    #define RG_RESSEL (u32)(0x3U << 20) //[21:20]
    #define RG_CAP_EN (u32)(0xFU << 16) //[19:16]
    #define RG_ADCVREFP (u32)(0x7U << 4) //[6:4]
    #define RG_FE_OFFSET_N (u32)(0x7U) //[2:0]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define REG_VGA_Normal_CFG1 (IO_ANA8_VGA+0x6E4U)
#else
#define REG_VGA_Normal_CFG1 (IO_ANA8_VGA + 0x748U)
#endif
    #define RG_CLAMP_MIDDLE_1 (u32)(0x1U << 31) //[31:31]
    #define RG_FE_OFFSET_P1 (u32)(0x7U << 28) //[30:28]
    #define RG_FE_1_PWD (u32)(0x1U << 27) //[27:27]
    #define RG_COPBIAS_1 (u32)(0x7U << 24) //[26:24]
    #define RG_COP_1_PWD (u32)(0x1U << 21) //[21:21]
    #define RG_CLAMP_GATE_1 (u32)(0x1U << 20) //[20:20]
    #define RG_MUXCAP_EN1 (u32)(0x1FU) //[4:0]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define REG_VGA_Normal_CFG2 (IO_ANA8_VGA+0x6E8U)
#else
#define REG_VGA_Normal_CFG2 (IO_ANA8_VGA + 0x74CU)
#endif
    #define RG_CLAMP_MIDDLE_2 (u32)(0x1U << 31) //[31:31]
    #define RG_FE_OFFSET_P2 (u32)(0x7U << 28) //[30:28]
    #define RG_FE_2_PWD (u32)(0x1U << 27) //[27:27]
    #define RG_COPBIAS_2 (u32)(0x7U << 24) //[26:24]
    #define RG_COP_2_PWD (u32)(0x1U << 21) //[21:21]
    #define RG_CLAMP_GATE_2 (u32)(0x1U << 20) //[20:20]
    #define RG_MUXCAP_EN2 (u32)(0x1FU) //[4:0]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define REG_VGA_Normal_CFG3 (IO_ANA8_VGA+0x6ECU)
#else
#define REG_VGA_Normal_CFG3 (IO_ANA8_VGA + 0x750U)
#endif
    #define RG_CLAMP_MIDDLE_3 (u32)(0x1U << 31) //[31:31]
    #define RG_FE_OFFSET_P3 (u32)(0x7U << 28) //[30:28]
    #define RG_FE_3_PWD (u32)(0x1U << 27) //[27:27]
    #define RG_COPBIAS_3 (u32)(0x7U << 24) //[26:24]
    #define RG_COP_3_PWD (u32)(0x1U << 21) //[21:21]
    #define RG_CLAMP_GATE_3 (u32)(0x1U << 20) //[20:20]
    #define RG_MUXCAP_EN3 (u32)(0x1FU) //[4:0]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define REG_VGA_Normal_CFG4 (IO_ANA8_VGA+0x6F0U)
#else
#define REG_VGA_Normal_CFG4 (IO_ANA8_VGA + 0x754U)
#endif
    #define RG_VGAADC_MON_SEL (u32)(0x3U << 28) //[29:28]
    #define RG_VGAADC1_IO_PWD (u32)(0x1U << 26) //[26:26]
    #define RG_VGAADC1_IGBIAS (u32)(0x3U << 24) //[25:24]
    #define RG_VGAADC2_IO_PWD (u32)(0x1U << 22) //[22:22]
    #define RG_VGAADC2_IGBIAS (u32)(0x3U << 20) //[21:20]
    #define RG_VGAADC3_IO_PWD (u32)(0x1U << 18) //[18:18]
    #define RG_VGAADC3_IGBIAS (u32)(0x3U << 16) //[17:16]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define REG_VGA_Normal_CFG5 (IO_ANA8_VGA+0x6F4U)
#else
#define REG_VGA_Normal_CFG5 (IO_ANA8_VGA + 0x758U)
#endif
    #define RG_VGAADC1_PHSEL (u32)(0x1U << 29) //[29:29]
    #define RG_VGAADC1_DC_EN (u32)(0x1U << 28) //[28:28]
    #define RG_VGAADC1_DIV_SEL (u32)(0x3U << 24) //[25:24]
    #define RG_VGAADC1_VSEL_EN (u32)(0x1U << 23) //[23:23]
    #define RG_VGAADC1_VSEL (u32)(0x7U << 20) //[22:20]
    #define RG_VGAADC1_CKSEL (u32)(0x1U << 19) //[19:19]
    #define RG_VGAADC1_CK_PWD (u32)(0x1U << 18) //[18:18]
    #define RG_VGAADC1_CORE_PWD (u32)(0x1U << 17) //[17:17]
    #define RG_VGAADC1_LSF_EN (u32)(0x1U << 16) //[16:16]
    #define RG_VGAADC1_REV0 (u32)(0xFFU << 8) //[15:8]
    #define RG_VGAADC1_REV1 (u32)(0xFFU) //[7:0]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define REG_VGA_Normal_CFG6 (IO_ANA8_VGA+0x6F8U)
#else
#define REG_VGA_Normal_CFG6 (IO_ANA8_VGA + 0x75CU)
#endif
    #define RG_VGAADC2_PHSEL (u32)(0x1U << 29) //[29:29]
    #define RG_VGAADC2_DC_EN (u32)(0x1U << 28) //[28:28]
    #define RG_VGAADC2_DIV_SEL (u32)(0x3U << 24) //[25:24]
    #define RG_VGAADC2_VSEL_EN (u32)(0x1U << 23) //[23:23]
    #define RG_VGAADC2_VSEL (u32)(0x7U << 20) //[22:20]
    #define RG_VGAADC2_CKSEL (u32)(0x1U << 19) //[19:19]
    #define RG_VGAADC2_CK_PWD (u32)(0x1U << 18) //[18:18]
    #define RG_VGAADC2_CORE_PWD (u32)(0x1U << 17) //[17:17]
    #define RG_VGAADC2_LSF_EN (u32)(0x1U << 16) //[16:16]
    #define RG_VGAADC2_REV0 (u32)(0xFFU << 8) //[15:8]
    #define RG_VGAADC2_REV1 (u32)(0xFFU) //[7:0]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define REG_VGA_Normal_CFG7 (IO_ANA8_VGA+0x6FCU)
#else
#define REG_VGA_Normal_CFG7 (IO_ANA8_VGA + 0x760U)
#endif
    #define RG_VGAADC3_PHSEL (u32)(0x1U << 29) //[29:29]
    #define RG_VGAADC3_DC_EN (u32)(0x1U << 28) //[28:28]
    #define RG_VGAADC3_DIV_SEL (u32)(0x3U << 24) //[25:24]
    #define RG_VGAADC3_VSEL_EN (u32)(0x1U << 23) //[23:23]
    #define RG_VGAADC3_VSEL (u32)(0x7U << 20) //[22:20]
    #define RG_VGAADC3_CKSEL (u32)(0x1U << 19) //[19:19]
    #define RG_VGAADC3_CK_PWD (u32)(0x1U << 18) //[18:18]
    #define RG_VGAADC3_CORE_PWD (u32)(0x1U << 17) //[17:17]
    #define RG_VGAADC3_LSF_EN (u32)(0x1U << 16) //[16:16]
    #define RG_VGAADC3_REV0 (u32)(0xFFU << 8) //[15:8]
    #define RG_VGAADC3_REV1 (u32)(0xFFU) //[7:0]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define REG_VGA_Normal_CFG8 (IO_ANA8_VGA+0x700U)
#else
#define REG_VGA_Normal_CFG8 (IO_ANA8_VGA + 0x764U)
#endif
    #define RG_RGB_REV (u32)(0xFFU << 8) //[15:8]
    #define RG_RELATCH_EN (u32)(0x1U << 4) //[4:4]
    #define RG_CLKOSEL_3 (u32)(0x1U << 3) //[3:3]
    #define RG_CLKOSEL_2 (u32)(0x1U << 2) //[2:2]
    #define RG_CLKOSEL_1 (u32)(0x1U << 1) //[1:1]
    #define RG_CLKINV_EN (u32)(0x1U) //[0:0]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define RGS_VGA_Normal_CFG0 (IO_ANA8_VGA+0x704U)
#else
#define RGS_VGA_Normal_CFG0 (IO_ANA8_VGA + 0x768U)
#endif
    #define RGS_YPBPR_ABUSE (u32)(0x1U << 31) //[31:31]

#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define PDWNC_VGACFG0 (IO_ANA8_VGA + 0x710U)
#else
#define PDWNC_VGACFG0 (IO_ANA8_VGA + 0x76CU)
#endif
    #define FLD_RG_VMUX_PWD (u32)(0x1U << 31) //[31:31]
    #define FLD_RG_RGB_EN (u32)(0x1U << 30) //[30:30]
    #define FLD_RG_HDTV1_EN (u32)(0x1U << 29) //[29:29]
    #define FLD_RG_HDTV0_EN (u32)(0x1U << 28) //[28:28]
    #define FLD_RGBHDTV01_EN (u32)(0x7U << 28)  //[30:28]   
    #define FLD_RG_SC_SCART (u32)(0x1U << 26) //[26:26]
    #define FLD_RG_DESPIKE (u32)(0x1U << 25) //[25:25]
    #define FLD_RG_AUTOBIAS_EN (u32)(0x1U << 24) //[24:24]
    #define FLD_RG_CVBSINVGA (u32)(0x3U << 20) //[21:20]
    #define FLD_RG_RGB_REVERSE (u32)(0x1U << 18) //[18:18]
    #define FLD_RG_HDTV1_REVERSE (u32)(0x1U << 17) //[17:17]
    #define FLD_RG_HDTV0_REVERSE (u32)(0x1U << 16) //[16:16]
    #define FLD_RG_HDMIIB_PWD (u32)(0x1U << 3) //[3:3]
    #define FLD_RG_VDACIB_PWD (u32)(0x1U << 2) //[2:2]
    #define FLD_RG_HSYNC_GPI_EN (u32)(0x1U << 1) //[1:1]
    #define FLD_RG_VSYNC_GPI_EN (u32)(0x1U) //[0:0]
    #define VGAMUX_RGB_EN (u32)4
    #define VGAMUX_HDTV1_EN (u32)2
    #define VGAMUX_HDTV0_EN (u32)1

#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define PDWNC_VGACFG1 (IO_ANA8_VGA + 0x714U)
#else
#define PDWNC_VGACFG1 (IO_ANA8_VGA + 0x770U)
#endif
    #define FLD_RG_VSYNC_EN (u32)(0x1U << 29) //[29:29]
    #define FLD_RG_HSYNC_EN (u32)(0x1U << 28) //[28:28]
    #define FLD_RG_CVBS_EN (u32)(0x1U << 27) //[27:27]
    #define FLD_RG_SCART_EN (u32)(0x7U << 24) //[26:24]
    #define FLD_RG_SOY1_EN (u32)(0x1U << 22) //[22:22]
    #define FLD_RG_SOY0_EN (u32)(0x1U << 21) //[21:21]
    #define FLD_SOY1_SOY0_EN (u32)(0x3U << 21) //[22:21]
    #define FLD_RG_SOG_EN (u32)(0x1U << 20) //[20:20]
    #define FLD_RG_SYNC_PWD (u32)(0x1U << 18) //[18:18]
    #define FLD_RG_SYNC_DESPK_EN (u32)(0x1U << 17) //[17:17]
    #define FLD_RG_FB_AB_EN (u32)(0x1U << 16) //[16:16]
    #define FLD_RG_BYPS_SYNCPROSR (u32)(0x1U << 13) //[13:13]
    #define FLD_RG_BYPS_SOGYPGA (u32)(0x1U << 12) //[12:12]
    #define FLD_RG_VGA_TEST_EN (u32)(0x1U << 11) //[11:11]
    #define FLD_RG_SYNC_TESTO_EN (u32)(0x1U << 10) //[10:10]
    #define FLD_RG_TSOGY_EN (u32)(0x1U << 9) //[9:9]
    #define FLD_RG_DIG_TST_EN (u32)(0x1U << 8) //[8:8]
    #define FLD_RG_SYNC_TSTSEL (u32)(0xFU << 4) //[7:4]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define PDWNC_VGACFG2 (IO_ANA8_VGA + 0x718U)
#else
#define PDWNC_VGACFG2 (IO_ANA8_VGA + 0x774U)
#endif
    #define FLD_RG_VREFMON (u32)(0xFFU << 24) //[31:24]
    #define FLD_RG_FEOUT_MONEN (u32)(0x1U << 23) //[23:23]
    #define FLD_RG_CLAMP_MONEN (u32)(0x1U << 22) //[22:22]
    #define FLD_RG_VGAVREF_MONEN (u32)(0x1U << 21) //[21:21]
    #define FLD_RG_VGA_TESTBUF_PWD (u32)(0x1U << 20) //[20:20]
    #define FLD_RG_VREF_CVBS (u32)(0x3U << 18) //[19:18]
    #define FLD_RG_SHIFT_PWD (u32)(0x1U << 17) //[17:17]
    #define FLD_RG_OFFCUR_PWD (u32)(0x1U << 16) //[16:16]
    #define FLD_RG_OFFCUR (u32)(0xFU << 12) //[15:12]
    #define FLD_RG_PSAV_REV (u32)(0xFFFU) //[11:0]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define PDWNC_VGACFG3 (IO_ANA8_VGA + 0x71CU)
#else
#define PDWNC_VGACFG3 (IO_ANA8_VGA + 0x778U)
#endif
    #define FLD_RG_SOGY_SINK (u32)(0x3FU << 24) //[29:24]
    #define FLD_RG_SOGY_SOURCE (u32)(0xFU << 20) //[23:20]
    #define FLD_RG_SOGY_SINK_PWD (u32)(0x1U << 17) //[17:17]
    #define FLD_RG_SOGY_SORS_PWD (u32)(0x1U << 16) //[16:16]
    #define FLD_RG_SOGY_RGAIN (u32)(0x3U << 12) //[13:12]
    #define FLD_RG_SOGY_BW (u32)(0x3U << 8) //[9:8]
    #define FLD_RG_CLAMPREFSEL (u32)(0x3U << 4) //[5:4]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define PDWNC_VGACFG4 (IO_ANA8_VGA + 0x720U)
#else
#define PDWNC_VGACFG4 (IO_ANA8_VGA + 0x77CU)
#endif
    #define FLD_RG_SYNCREV1 (u32)(0xFFFFU << 16) //[31:16]
    #define FLD_RG_SYNC1_VTH (u32)(0xFU << 12) //[15:12]
    #define FLD_RG_SYNC1_VTL (u32)(0xFU << 8) //[11:8]
    #define FLD_RG_SYNC2_VTH (u32)(0xFU << 4) //[7:4]
    #define FLD_RG_SYNC2_VTL (u32)(0xFU) //[3:0]
    #define FLD_RG_SYNC2_VHLSEL (u32)(0xFFU) //[7:0]
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define PDWNC_VGACFG5 (IO_ANA8_VGA + 0x724U)
#else
#define PDWNC_VGACFG5 (IO_ANA8_VGA + 0x780U)
#endif
    #define FLD_RG_SYNCREV2 (u32)(0xFFFFU) //[15:0]

/**
YPbPr_VGA_EFUSE
*/
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define YPBPR_VGA_EFUSE_BASE (IO_BASE_VA+0x54000U)
#else
#define YPBPR_VGA_EFUSE_BASE (IO_BASE_REG_VA + 0x54000U)
#endif

#define YPBPR_VGA_EFUSE (YPBPR_VGA_EFUSE_BASE + 0x66CU)
    #define YPBPR_VGA_EFUSE_GAIN (u32)(0xFFFFFFU)


#endif

