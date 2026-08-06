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

#ifndef TVD_HW_REG_H
#define TVD_HW_REG_H

#ifndef __ARM2__
//#include <asm/io.h>
#endif
extern unsigned long  IO_VBASE_VA;

/* **********************************************************************/
/*IRQ. Interrupt Registers define.*/
#define TVD_INTR_EN         0x40CU          /* SYS_03, Interrupt mask*/
#define TVD_INTR_STA        0x000U          /* INT_COLLECT, Interrupt Status*/
#define INTR_WFF_VSYNC_TVD      (1U << 22)/*(1 <<22 for 3360)*/
#define INTR_VSYNC_TVD          (1U << 9)
#define INTR_TIMERA_TVD         (1U << 3)    /* TVD timerA interrupt, write 1 to clear*/
#define INTR_MODE_TVD           (1U << 1)    /* TVD mode switching, write 1 to clear*/
#define INTR_VPRES_TVD          (1U << 0)    /* TVD video on/off switching, write 1 to clear    */

#define STA_REG10           0x040U /* DFE_STA_00*/
#define VPRES_OFF            (1U << 1)    /* VPRES_OFF: loss signal*/
#define VPRES_ON            (1U << 0)    /* VPRES_ON : get signal*/


/* **********************************************************************/
/*STA. Status Registers define. Registers are  Read-only.*/
#define REG_STA_REG11    0x044U        /* DFE_STA_01*/
#define BLANK_LV        (0x3FFU << 0)
#define REG_STA_REG13    0x04CU        /* DFE_STA_03*/
#define PY_LV             (0x3FFU << 0)
#define REG_STA_REG14    0x050U        /* DFE_STA_04*/
#define AGAIN_CODE        (0x7FU << 8)
#define REG_STA_REG15    0x054U        /* DFE_STA_05*/
#define CP_CUR            (0x7FU << 16)
/* Register Macro Declaration*/
#define wHwTvdCPCur()    ((TVD_READ32(REG_STA_REG15) & CP_CUR) >> 16)

#define REG_STA_REG18    0x060U        /* DFE_STA_08*/
#define COCH_DETECTED    (1U << 0)
/* Register Macro Declaration*/
#define fgHwTvdCoChannel(tvd_base)    ((TVD_READ32(tvd_base + REG_STA_REG18) & COCH_DETECTED))

#define REG_STA_CDET_00     0x080U    /* Mode Detection Status. Read-only*/
#define CKILL               (1U << 31)
#define MODE_TVD3D          (0x7U << 28)    /*30:28*/
#define BLOCK4DET           (1U << 27)
#define V525_TVD3D          (1U << 26)
#define IS443_TVD3D         (1U << 25)
#define PHALT_TVD3D         (1U << 24)
#define FH_NEG              (1U << 23)
#define FH_POS              (1U << 22)
#define STD_V625            (1U << 21)
#define STD_V525            (1U << 20)
#define NSTD_V625           (1U << 19)
#define NSTD_V525           (1U << 18)
#define NARRSYNC            (1U << 17)
#define SB4CTG_FLAG         (1U << 16)
#define PALSW_TGL_FLAG      (1U << 15)
#define SVF_BSTDET_F        (1U << 14)
#define VPRES_SVF           (1U << 13)
#define VPRES_TVD3D         (1U << 12)
#define SCF                 (1U << 11)
#define PH_OLD              (1U << 10)
#define NA_STATE            (0x3U << 8)    /*9:8*/
#define NR_LEVEL            (0xFFU << 0)    /*7:0*/
/* Register Macro Declaration*/
#define fgHwTvdCKill()         ((TVD_READ32(REG_STA_CDET_00) & CKILL) >> 31)
#define bHwTvdMode(tvd_base)           ((TVD_READ32(tvd_base + REG_STA_CDET_00) & MODE_TVD3D) >> 28)
#define fgHwTvd525()           ((TVD_READ32(REG_STA_CDET_00) & V525_TVD3D) >> 26)
#define fgHwTvd443()           ((TVD_READ32(REG_STA_CDET_00) & IS443_TVD3D) >> 25)
#define fgHwTvdFHNeg(tvd_base)           ((TVD_READ32(tvd_base + REG_STA_CDET_00) & FH_NEG) >> 23)
#define fgHwTvdFHPos(tvd_base)           ((TVD_READ32(tvd_base + REG_STA_CDET_00) & FH_POS) >> 22)
#define fgHwTvdVPresSVF()      ((TVD_READ32(REG_STA_CDET_00) & VPRES_SVF) >> 13)
#define fgHwTvdVPresTVD3D(tvd_base)    ((TVD_READ32(tvd_base + REG_STA_CDET_00) & VPRES_TVD3D) >> 12)
#define bHwTvdNAState(tvd_base)        ((TVD_READ32(tvd_base + REG_STA_CDET_00) & NA_STATE) >> 8)
#define bHwTvdNRLevel()        ((TVD_READ32(REG_STA_CDET_00) & NR_LEVEL))

#define REG_STA_REG22    0x088U        /* TG_STA_00 */
#define HEAD_SWITHC      (1U << 19)
#define TRICK            (1U << 15)
/* Register Macro Declaration*/
#define fgHwTvdHeadSwitch(tvd_base)     ((TVD_READ32(tvd_base + REG_STA_REG22) & HEAD_SWITHC) >> 19)
#define fgHwTvdTrick(tvd_base)          ((TVD_READ32(tvd_base + REG_STA_REG22) & TRICK) >> 15)

#define REG_STA_REG23    0x08CU        /* TG_STA_01*/
#define AVG_VLEN        (0x3FFU << 0)
/* Register Macro Declaration*/
#define wHwTvdAvgVLen(tvd_base)            ((TVD_READ32(tvd_base + REG_STA_REG23) & AVG_VLEN))

#define REG_STA_REG24    0x090U        /* STA_TG_02*/
#define VCR_BV           (1U << 26)
/* Register Macro Declaration*/
#define fgHwTvdVCRBV()            ((TVD_READ32(REG_STA_REG24) & VCR_BV) >> 26)

#define REG_STA_REG2A    0x0A8U        /* STA_CTG_04*/
#define ADC_CODE         (0xFFFU << 0)

#define REG_STA_REG2B    0x0ACU
#define VAR_CVBS_CLIP    (0x3FFFFU << 0)

#define REG_STA_REG2D    0x0B4U        /* STA_TG_05*/
#define LINE_STDFH_FLAG  (1U << 29)
#define STA_LCNT         (0xFFFU << 0)
/* Register Macro Declaration*/
#define fgHwTvdLineSTDFH()        ((TVD_READ32(REG_STA_REG2D) & LINE_STDFH_FLAG) >> 29)
#define wHwTvdAvgLineCnt(tvd_base)        ((TVD_READ32(tvd_base + REG_STA_REG2D) & STA_LCNT))

/* **********************************************************************/
/*SYS. System Registers define*/
#define REG_SYS_00_RST_CTRL            0x400U        /* Reset Control*/
#define REG_SYS_01_COLOR_PRO0          0x404U        /* TVD Color process0*/
#define COLOR_PRO_YGAIN(x)             (((x) & 0xFFU) << 0)    /* [7:0], High 4bit is integer, Low 4bit is decimal.*/
#define COLOR_PRO_YOFFSET(x)           (((x) & 0xFFU) << 8)    /* [15:8], signed integer*/
#define COLOR_PRO_UCOSGAIN(x)          (((x) & 0xFFU) << 16)    /* [23:16], High 4bit is integer, Low 4bit is decimal.*/
#define COLOR_PRO_VCOSGAIN(x)          (((x) & 0xFFU) << 24)    /* [31:24], High 4bit is integer, Low 4bit is decimal.*/
#define REG_SYS_02_COLOR_PRO1          0x408U        /* TVD Color process1*/
#define COLOR_PRO_USINGAIN(x)          (((x) & 0xFFU) << 0)    /* [7:0], High 4bit is integer, Low 4bit is decimal.*/
#define COLOR_PRO_VSINGAIN(x)          (((x) & 0xFFU) << 8)    /* [15:8], High 4bit is integer, Low 4bit is decimal.*/
#define COLOR_PRO_UOFFSET(x)           (((x) & 0xFFU) << 16)    /* [23:16], signed integer*/
#define COLOR_PRO_VOFFSET(x)           (((x) & 0xFFU) << 24)    /* [31:24], signed integer*/

/* **********************************************************************/
/*VSRC Registers define*/
#define REG_VSRC_07         0x434U    /* VSRC_CONTROL_7*/
#define AAF_SEL 	        (7U << 17) //19:17
#define RG_VSRC_INV_AIDX    (1U << 8)

#define REG_VSRC_08        0x438U    /* CCI_CONTROL_8*/
#define VPRES_COCH_EN      (1U << 16)

/* **********************************************************************/
/*VFE.CVBS (Analog) Registers define*/
#define REG_VFE_00         0x440U
#define RG_MIXINPUT        (1U << 31)
#define RG_CVBS_PWD        (1U << 30)
#define RG_MIX_PWD         (1U << 29)
#define RG_PGABUFNA_PWD    (1U << 28)
#define RG_OFFCURA_PWD     (1U << 27)
#define RG_PROT_PWD        (1U << 26)
#define RG_INMUX_PWD       (1U << 25)
#define RG_SHIFTA_PWD      (1U << 24)
#define RG_CLAMP_PWD       (1U << 23)
#define RG_OFFCURA         (0x1FU << 18)

/* CVBS CHA PGA CM buffer input select, 0: SC mode(ac83xx don't support), 1: SY mode   */
#define RG_VAGSELA         (1U << 17)
#define RG_UPDN            (1U << 16)    /* CVBS enable clamp on blank for CHA, 0: disable, 1: enable*/
#define RG_AAF_BW          (0x3U << 13)     /* CVBS AAF bandwidth.*/
#define RG_AISEL           (0xFU << 8)
#define RG_VIDEOBYPASS     (0xFFU << 0)
#define REG_VFE_01         0x444U
#define RG_CVBSADC_PWD     (1U << 29)
#define RG_MIXVOCM_SEL     (3U << 2)     /* 00: 1.65V, 01:1.4V, 10: 1V, 11: 755mV.*/
#define RG_MIXGAIN         (3U << 0)        /* 00: 2x, 01:1.2x, 10: 1.4x, 11: 1.65x.*/
#define REG_VFE_02         0x448U
#define RG_CVBSADC_SEL_CKPLL    (1U << 14)     /* 0: Select clock source form PLL, 1: Select clock source form XTAL.*/
#define RG_CVBS_REV_2            (1U << 2)


#define REG_VFE_03         0x44c
#define RG_VAGSELB         (1 << 14)
#define RG_BTM_EN          (1 << 12)
#define RG_PGABUFNB_PWD    (1 << 7)
#define RG_SHIFTB_PWD      (1 << 6)
#define RG_OFFCURB_PWD     (1 << 5)

/* **********************************************************************/

/*DFE. Digital Front-End Registers define*/
#define REG_DFE_01    0x4C4U
#define BLANK_WIN_START        (0xFFU << 0)
/*----------DFE Blank Level Related Define --------------------//*/
#define DFE_BLANK_WIN_START_STD_L      0x45U  /*Blank Window Start in Standard line*/
#define DFE_BLANK_WIN_START_NSTD_L     0x3FU  /*Blank Window Start in Non Standard line*/
#define REG_DFE_02                     0x4C8U
#define Y4H_BW                         (0x3U << 0)
#define REG_DFE_03                     0x4CCU
#define AGC2_MODE                      (0xFU << 28)
#define AGC2_PK_MODE                   (0xFU << 24)
#define REG_DFE_07                     0x4DCU
#define AGC2_MANUAL_ACODE              (0x7FU << 16)
#define REG_DFE_08                     0x4E0U
#define DCLAMP_Y_EN                    (1U << 29)
#define REG_DFE_0A                     0x4E8U            /* Digital Front-End Control 0A*/
#define CLAMP_TARGET_BLANK_LV          (0x3FFU << 10)
#define REG_DFE_0E                     0x4F8U            /* Digital Front-End Control 0E*/
#define VPRES_EN                       (1U << 31)
#define VPRES_FORCE_ON                 (1U << 30)
#define MVPRES_TVD_EN                  (1U << 29)
#define MVPRES_CLAMP_EN                (1U << 28)
#define MVPRES_AGC_EN                  (1U << 27)
#define MVPRES_BLANK_EN                (1U << 26)
#define MVPRES_SYNC_EN                 (1U << 25)
#define MVPRES_HDET_EN                 (1U << 24)
#define MVPRES_VDET_EN                 (1U << 23)
#define MVPRES_TVD                     (1U << 22)
#define MVPRES_CLAMP                   (1U << 21)
#define MVPRES_AGC                     (1U << 20)
#define MVPRES_BLANK                   (1U << 19)
#define MVPRES_SYNC                    (1U << 18)
#define MVPRES_HDET                    (1U << 17)
#define MVPRES_VDET                    (1U << 16)
#define DCLAMP_UP_LIM(x)               (((x) & 0xFFU) << 8)/*15:8*/
#define DCLAMP_CHECK_LIM               (1U << 7)
#define NR_DET_VPRES_SEL               (1U << 6)
#define VPRES4TVD_MODE                 (1U << 5)
#define VPRES4PIC_MODE                 (1U << 4)
#define VPRES_SEL                      (1U << 3)
#define VPRES_MASK(x)                  (((x) & 0x7U) << 0) /*2:0*/

#define REG_DFE_19     0x524U

/* gain control module selection. 0: old AGC support CHA/CHB, 1: new AGC, only support CHB*/
#define AGC_SEL        (1U << 26)
#define REG_DFE_1F     0x53CU        /* CLAMP_TABLE_05    */
#define REG_DFE_21     0x7ECU
#define AGC_MLAGC_EN   (1U << 30)    /* enable manual analog gain. 1: enable, 0: disable*/
#define REG_DFE_22     0x7F0U
#define AGC_MAGC       (0xFFU << 16)    /* manual analog gain code.*/



/* **********************************************************************/
/*CDET. Chroma Detection Register Define*/
#define REG_CDET_00         0x540U        /* Mode Detection 0*/
#define BST_DET_ADAP        (1U << 31)
#define DET443_SEL_ADAP     (1U << 30)
#define PALSW_FAST_ADAP     (1U << 29)
#define CTG_ADAP            (1U << 28)
#define CDET_START_ADAP     (1U << 27)
#define CKILL_ADAP          (1U << 26)
#define PALSW_ADAP          (1U << 25)
#define CAGC_ADAP           (1U << 24)
#define CKILL_SEL(x)        (((x) & 0x3U) << 22)    /*23:22*/
#define MODE_CKILL_EN       (1U << 21)
#define SCF_SEL             (1U << 20)
#define MDET_V525_SEL       (1U << 19)
#define MDET_SCF_EN         (1U << 18)
#define NR_OUT_SEL(x)       (((x) & 0x3U) << 16)    /*17:16*/
#define DET443_SEL          (1U << 15)
#define HN_DET443_EN        (1U << 14)
#define NTSC443_EN          (1U << 13)
#define PALN_EN             (1U << 12)
#define PALM_EN             (1U << 11)
#define PAL60_EN            (1U << 10)
#define SECAM_EN            (1U << 9)
#define INI_IS443           (1U << 8)
#define INI_PHALT           (1U << 7)
#define INI_V525            (1U << 6)
#define L525                (1U << 5)
#define MODE000             (1U << 4)
#define TVD_MMODE           (1U << 3)
#define TVD_MODE(x)         (((x) & 0x7U) << 0)    /*2:0    */
#define REG_CDET_04         0x550U        /* Chorma detection 0    */
#define CTG_NA_SEL          (1U << 29)
#define CDET_NA_SEL         (1U << 28)

/* **********************************************************************/
/*TG. Timing Generation Register define*/
#define REG_TG_04           0x590U
#define LF_OFFSET_EN        (1U << 12)
#define REG_TG_08           0x5A0U
#define VALIGN_SPEED(x)     (((x) & 0x3U) << 30)    /*31:30*/
#define SKIP_VSPIKE         (1U << 29)
#define FAST_VALIGN         (1U << 28)
#define TGEN_DEBUG          (1U << 27)
#define HVDET_FIXBLK        (1U << 26)
#define FAST_VLOCK          (1U << 25)
#define LLOCK_GUARD         (1U << 24)
#define TKMODE_SEL          (1U << 23)
#define TKMODE_THR(x)       (((x) & 0xFU) << 19)    /*22:19*/
#define TVD_VCR_EN          (1U << 18)
#define FDET_SEL            (1U << 16)
#define VFIR_EN             (1U << 15)
#define VFIR_SEL(x)         (((x) & 0x3U) << 12)    /*13:12*/
#define NARRSYNC_TH(x)      (((x) & 0xFU) << 8)    /*11:8*/
#define LCNT_DLY(x)         (((x) & 0xFFU))    /*7:0*/

#define REG_TG_0B           0x5ACU        /* TVD Gen-Lock*/
#define AUTO_AVDELAY        (1U << 31)
#define AUTO_MLLOCK         (1U << 30)
#define AUTO_LF_OFFSET      (1U << 29)
#define AUTO_FAST_KP_GAIN   (1U << 28)
#define AUTO_FASTV          (1U << 27)
#define AUTO_FASTV_2S       (1U << 26)
#define TG_VPRES_FORCE(x)   (((x) & 0x3U) << 24)    /*25:24*/
#define L525_FORCE(x)       (((x) & 0x3U) << 22)    /*23:22*/
#define LOCKVLEN_FORCE(x)   (((x) & 0x3U) << 20)    /*21:20*/
#define DEF_VLEN_SEL        (1U << 19)
#define TVD_PATGEN_EN       (1U << 18)
#define TVD_PATGEN_MODE(x)  (((x) & 0x3U)  << 16)    /*17:16*/
#define IIR_SLICE_LIM(x)    (((x) & 0xFFU) << 8)    /*15:8*/
#define HVDET_MBLK(x)       (((x) & 0xFFU) << 0)    /*7:0*/
#define REG_TG_0D           0x5B4U        /* TVD Gen-Lock D*/
#define MXSD_FHNEG_EN       (1U << 15)    /* Positive line frequency tolerance enable*/
#define HLEN_FHPOS_EN       (1U << 14)    /* Negative line frequency tolerance enable*/

#define TG_17               0x5DCU        /* TVD Gen-Lock 17*/
#define FRAME_STDFV_TH(x)   (((x) & 0XFFU) << 24)  /*31:24*/
#define FRAME_STDFH_TH(x)   (((x) & 0XFFU) << 16)  /*23:16*/
#define LINE_STDFH_UP(x)    (((x) & 0x7U) << 13)    /*15:13*/
#define LINE_STDFH_LO(x)    (((x) & 0x7U) << 10)    /*12:10*/
#define COCH_HSS_EN         (1U << 9)
#define HSS_FORCE_VLOCKVLD  (1U << 8)
#define HSS_FORCE_HLOCKVLD  (1U << 7)
#define HSS_FORCE_TRICKVLD  (1U << 6)
#define KIP_FORCE_EN        (1U << 5)
#define MHSS_EN             (1U << 4)
#define MHSS                (1U << 3)

#define CDET_0C 0x570U

#define CAGC_MAX(x)    (((x) & 0XFFU) << 24) /*31:24*/
#define CAGC_MIN       (((x) & 0XFU) << 20) /*23:20*/
#define BWSTVMASK_IGNR (1U << 19) /*19*/
#define BSWTGT_IGNR    (1U << 18) /*18*/
#define CAGCVMASK_IGNR (1U << 17) /*17*/
#define CAGCGT_IGNR    (1U << 16)
#define PHALT_NB_EN    (1U << 15) /*15*/
#define DET443_NB_EN   (1U << 14) /*14*/
#define NR_DC_SEL      (1U << 13) /*13*/
#define NR_DIFF_SEL    (1U << 12) /*12*/
#define NR_IIR_PERIOD  (1U << 11) /*11*/
#define NR_IIR_EN      (1U << 10) /*10*/
#define NR_GAIN        (((x) & 0X3U) << 8) /*9:8*/
#define NR_HSEL        (((x) & 0X3U) << 6) /*7:6*/
#define NR_VSRC_SEL    (1U << 5)  /*5*/
#define NR_VSEL        (((x) & 0X1FU) << 0)/*4:0*/

#define REG_CTG_00            0x5E0U
#define SOBVLD_MASK_EN        (1U << 26)
#define CTG_SWLBF             (1U << 21)
#define REG_CTG_05            0x5F4U        /* Chroma Timing Generation 5*/
#define BST_START_SEL         (1U << 30)    /* Burst demod window start point selection*/

/* **********************************************************************/
/*SECAM. SECAM Register define*/
#define REG_SECAM_07        0x61CU

/* Output selection for YUV measure. 0: ADC Y/C, 1: DAGC Y/C, 2: 4FSC Y/C, 3:YCSep Y/C,4: Demod Y/C, */
#define YUV_CATCH_SEL       (0xFU << 28)
/* 5: Demod/SECAM Y/U/V, 6: YC Delay Y/U/V, 7: BLCOMP Y/U/V, 8: TVD Output Y/Cb/Cr*/
/* **********************************************************************/

// 3365 add 

#define REG_CVBS_CFG0 	0x6f8
    #define RG_CVBS_PWD_CH0             (0x1<<0)
    #define RG_CLAMP_PWD_CH0            (0x1<<7)
    #define RG_UPDN_CH0                 (0x1<<14)
    #define RG_VAGSELA_CH0              (0x1<<13)
    #define RG_PGABUFNA_PWD_CH0         (0x1<<2)
    #define RG_SHIFTA_PWD_CH0           (0x1<<6)
    #define RG_OFFCUROA_PWD_CH0         (0x1<<3)
    #define RG_AISEL_CH0                (0xf<<20)
    #define RG_VIDEOBYPASS_CH0          (0xff<<24)
    #define RG_INMUX_PWD_CH0            (0x1<<5)


#define REG_CVBS_CFG2 	0x700
    #define RG_CVBSADC_PWD_CH0             (0x1<<6)
    #define RG_CVBS0P_CHA_SEL_CH0          (0x1<<13)
    #define RG_AISEL_CH0                   (0xf<<20)


#define REG_CVBS_CFG3 	0x704
    #define RG_BTM_EN_CH0       (0x1<<3)
    #define RG_VAGSELB_CH0      (0x1<<1)
    #define RG_PGABUFNB_PWD_CH0 (0x1<<8)
    #define RG_SHIFTB_PWD_CH0  (0x1<<9)
    #define RG_OFFCUROB_PWD_CH0 (0x1<<10)
    #define RG_OFFCUROB_CH0 (0x1f<<11)
    #define RG_CVBSADC_SEL_CKPLL_CH0 (0x1<<30)


#define REG_CVBS_CFG4 	0x708
    #define RG_CLAMP_PWD_CH1            (0x1<<1)
    #define RG_UPDN_CH1                 (0x1<<8)
    #define RG_VAGSELA_CH1              (0x1<<7)
    #define RG_SHIFTA_PWD_CH1           (0x1<<0)
    #define RG_AISEL_CH1                (0xf<<14)
    #define RG_VIDEOBYPASS_CH1          (0xff<<18)



#define REG_CVBS_CFG5 	0x70c
    #define RG_CVBS_PWD_CH1             (0x1<<26)
    #define RG_PGABUFNA_PWD_CH1         (0x1<<28)
    #define RG_OFFCUROA_PWD_CH1         (0x1<<29)
    #define RG_INMUX_PWD_CH1            (0x1<<31)


#define REG_CVBS_CFG6 	0x710
    #define RG_CVBS0P_CHA_SEL_CH1          (0x1<<6)

    

#define REG_CVBS_CFG7 	0x714
    #define RG_CVBSADC_PWD_CH1             (0x1<<31)
    #define RG_PGABUFNB_PWD_CH1            (0x1<<1)
    #define RG_SHIFTB_PWD_CH1              (0x1<<2)
    #define RG_OFFCUROB_PWD_CH1            (0x1<<3)
    #define RG_CVBSADC_SEL_CKPLL_CH1       (0x1<<23)


#define REG_CVBS_CFG8 	0x718
    #define RG_BTM_EN_CH1               (0x1<<27)
    #define RG_VAGSELB_CH1              (0x1<<25)
    #define RG_UPDN_CH2                 (0x1<<0)
    #define RG_AISEL_CH2                (0xf<<6)
    #define RG_VIDEOBYPASS_CH2          (0xff<<10)

//
#define REG_CVBS_CFG9 	0x71c
    #define RG_CVBS_PWD_CH2             (0x1<<18)
    #define RG_CLAMP_PWD_CH2            (0x1<<25)
    #define RG_VAGSELA_CH2              (0x1<<31)
    #define RG_PGABUFNA_PWD_CH2         (0x1<<20)
    #define RG_OFFCUROA_PWD_CH2         (0x1<<21)
    #define RG_INMUX_PWD_CH2            (0x1<<23)

#define REG_CVBS_CFG11 	0x724
    #define RG_CVBSADC_PWD_CH2          (0x1<<24)
    #define RG_CVBS0P_CHA_SEL_CH2       (0x1<<31)
    #define RG_CVBSADC_SEL_CKPLL_CH2    (0x1<<16)

    
#define REG_CVBS_CFG12 	0x728
    #define RG_SHIFTA_PWD_CH2           (0x1<<25)
    #define RG_BTM_EN_CH2               (0x1<<19)
    #define RG_VAGSELB_CH2              (0x1<<17)
    #define RG_PGABUFNB_PWD_CH2         (0x1<<24)
    #define RG_SHIFTB_PWD_CH2           (0x1<<25)
    #define RG_OFFCUROB_PWD_CH2         (0x1<<26)
    #define RG_VIDEOBYPASS_CH3          (0xff<<2)


#define REG_CVBS_CFG13 	0x72c
    #define RG_CVBS_PWD_CH3             (0x1<<8)
    #define RG_CLAMP_PWD_CH3            (0x1<<15)
    #define RG_UPDN_CH3                 (0x1<<22)
    #define RG_VAGSELA_CH3              (0x1<<21)
    #define RG_PGABUFNA_PWD_CH3         (0x1<<10)
    #define RG_SHIFTA_PWD_CH3           (0x1<<14)
    #define RG_OFFCUROA_PWD_CH3         (0x1<<11)
    #define RG_AISEL_CH3                (0xf<<28)
    #define RG_INMUX_PWD_CH3            (0x1<<13)

           
#define REG_CVBS_CFG15 	0x734
    #define RG_CVBSADC_PWD_CH3          (0x1<<14)
    #define RG_CVBS0P_CHA_SEL_CH3       (0x1<<21)
    #define RG_CVBSADC_SEL_CKPLL_CH3    (0x1<<6)


#define REG_CVBS_CFG16 	0x738
    #define RG_BTM_EN_CH3               (0x1<<11)
    #define RG_VAGSELB_CH3              (0x1<<9)
    #define RG_PGABUFNB_PWD_CH3         (0x1<<16)
    #define RG_SHIFTB_PWD_CH3           (0x1<<17)
    #define RG_OFFCUROB_PWD_CH3         (0x1<<18)

/* **********************************************************************/
/*Comb Filter*/
#define IO_COMB_BASE (IO_VBASE_VA + 0x5b000)
//Page COMB_1
#define STA_COMB_00 (IO_COMB_BASE + 0x0C0)
    #define BIN12PIXCNTSTA TDC_FIELD(32,0)//[31:0]
    
#define STA_COMB_01 (IO_COMB_BASE + 0x0C4)
    #define BIN34PIXCNTSTA TDC_FIELD(32,0)//[31:0]
    
#define STA_COMB_02 (IO_COMB_BASE + 0x0C8)
    #define CZPCNTSTA TDC_FIELD(32,0)//[31:0]
    
#define STA_COMB_03 (IO_COMB_BASE + 0x0CC)
    #define NOISTIPISUM TDC_FIELD(32,0)//[31:0]
    
#define STA_COMB_04 (IO_COMB_BASE + 0x0D0)
	#define CHROMAFLICKERFIELDCNT TDC_FIELD(4,28)//[31:28]
    #define PERFIELDCHROMAFLICKERCNT TDC_FIELD(8,20)//[27:20]
    #define INPIXCNTSTA TDC_FIELD(20,0)//[19:0]
    
#define STA_COMB_05 (IO_COMB_BASE + 0x0D4)
    #define PERMOTIONPIXCOUNT_FIELD TDC_FIELD(1,31)//[31:31]
    #define PERMOTIONPIXCOUNT TDC_FIELD(8,20)//[27:20]
    #define MOPIXCNTSTA TDC_FIELD(20,0)//[19:0]
    
#define STA_COMB_06 (IO_COMB_BASE + 0x0D8)
    #define PERFIELDCHROMAVARSUM TDC_FIELD(8,20)//[27:20]
    #define MBPIXCNTSTA TDC_FIELD(20,0)//[19:0]
    
#define STA_COMB_07 (IO_COMB_BASE + 0x0DC)
    #define LUMASUMSTA TDC_FIELD(32,0)//[31:0]
    
#define STA_COMB_08 (IO_COMB_BASE + 0x0E0)
    #define COLORSUMSTA TDC_FIELD(32,0)//[31:0]
#define STA_COMB_09 (IO_COMB_BASE + 0x0E4)
    #define LUMAEDGESTA TDC_FIELD(32,0)//[31:0]
    
#define STA_COMB_0A (IO_COMB_BASE + 0x0E8)
    #define LUMAEDPISTA TDC_FIELD(32,0)//[31:0]
    
#define STA_COMB_0B (IO_COMB_BASE + 0x0EC)
    #define COLOREDGESTA TDC_FIELD(32,0)//[31:0]
    
#define STA_COMB_0C (IO_COMB_BASE + 0x0F0)
    #define FIFO_EVERFULL TDC_FIELD(1,22)//[22:22]
    #define FIFOEMPTY TDC_FIELD(1,21)//[21:21]
    #define FIFOEMPTY_A TDC_FIELD(1,20)//[20:20]
    #define FIFOEMPTY_AA TDC_FIELD(1,19)//[19:19]
    #define BIG_MOTION TDC_FIELD(1,18)//[18:18]
    #define NONVCR3D TDC_FIELD(1,17)//[17:17]
    #define NONVCRTG TDC_FIELD(1,16)//[16:16]
    #define SMALL_MOTION TDC_FIELD(1,15)//[15:15]
    #define STILL_FRAME TDC_FIELD(1,14)//[14:14]
    #define WHOLE_FRAME_C3D_SMALL TDC_FIELD(1,13)//[13:13]
    #define NOISEFG TDC_FIELD(1,4)//[4:4]
    #define CZP_DETECT TDC_FIELD(1,3)//[3:3]
    #define SWEEP_DETECTION TDC_FIELD(1,2)//[2:2]
    #define WHOLE_FRAME_VERTICAL_SMOOTH TDC_FIELD(1,1)//[1:1]
    #define NOISY_SMALL TDC_FIELD(1,0)//[0:0]
    
#define COMB_CTRL_00 (IO_COMB_BASE + 0x640)
    #define CLRFULL TDC_FIELD(1,28)//[28:28]
    #define CLREMPTY TDC_FIELD(1,27)//[27:27]
    #define FULLSTA TDC_FIELD(1,26)//[26:26]
    #define EMPTYSTA TDC_FIELD(1,25)//[25:25]
    #define EN_BLOCK_BY_SEED TDC_FIELD(1,24)//[24:24]
    #define ADYSEL TDC_FIELD(1,23)//[23:23]
    #define SETZERO TDC_FIELD(1,22)//[22:22]
    #define FIFORY TDC_FIELD(6,16)//[21:16]
    #define DML_METHOD TDC_FIELD(1,15)//[15:15]
    #define DML_FSEL TDC_FIELD(1,14)//[14:14]
    #define FIFORX TDC_FIELD(6,8)//[13:8]
    #define DMEGSEL TDC_FIELD(1,7)//[7:7]
    #define DMTPSEL TDC_FIELD(1,6)//[6:6]
    #define FIFORM1 TDC_FIELD(6,0)//[5:0]
    
#define COMB_CTRL_01 (IO_COMB_BASE + 0x644)
    #define WVSYNCTH TDC_FIELD(7,25)//[31:25]
    #define DRAMBASEADR TDC_FIELD(25,0)//[24:0]
		
#define COMB_CTRL_02 (IO_COMB_BASE + 0x648)
    #define EN3D TDC_FIELD(1,31)//[31:31]
    #define ENFORCE3D TDC_FIELD(1,30)//[30:30]
    #define COMB_OPTION TDC_FIELD(2,28)//[29:28]
    #define B10MODE TDC_FIELD(1,27)//[27:27]
    #define FORCEDRAM TDC_FIELD(1,26)//[26:26]
    #define GAMEMODE TDC_FIELD(1,25)//[25:25]
    #define ENPGMODE TDC_FIELD(1,24)//[24:24]
    #define WITHCLRTH_0 TDC_FIELD(1,23)//[23:23]
    #define ENVCRFSIG2 TDC_FIELD(1,22)//[22:22]
    #define ENVCRFSIG1 TDC_FIELD(1,21)//[21:21]
    #define HCNT3D TDC_FIELD(10,11)//[20:11]
    #define HLEN3D TDC_FIELD(11,0)//[10:0]
		
#define COMB_CTRL_03 (IO_COMB_BASE + 0x64C)
    #define VLINEST TDC_FIELD(7,25)//[31:25]
    #define VLINECNT TDC_FIELD(9,16)//[24:16]
	#define DUMPSTAL TDC_FIELD(1,15)//[15:15]
    #define ENCKILLWFIFO TDC_FIELD(1,14)//[14:14]
    #define LBUFSEL TDC_FIELD(2,12)//[13:12]
    #define DUMPSEL TDC_FIELD(4,8)//[11:8]
    #define ICTST TDC_FIELD(1,7)//[7:7]
    #define ICTSTLOCK TDC_FIELD(1,6)//[6:6]
    #define HOLD TDC_FIELD(1,5)//[5:5]
    #define FSKBACK TDC_FIELD(1,4)//[4:4]
	#define ENSHORT TDC_FIELD(1,3)//[3:3]
    #define CLRSRAM TDC_FIELD(1,2)//[2:2]
    #define ENYCCKILL TDC_FIELD(1,1)//[1:1]
    #define ENCKILL TDC_FIELD(1,0)//[0:0]
    
#define COMB_CTRL_04 (IO_COMB_BASE + 0x650)
    #define CRCSEL TDC_FIELD(1,31)//[31:31]
    #define DUMPNOW TDC_FIELD(1,27)//[27:27]
    #define HLEN3D_M TDC_FIELD(11,16)//[26:16]
    #define WVSYNCTH_0 TDC_FIELD(1,15)//[15:15]
    #define ENDUMPENDADR TDC_FIELD(1,14)//[14:14]
    #define ADB4SEL TDC_FIELD(2,12)//[13:12]
    #define AD10BITMODE TDC_FIELD(1,11)//[11:11]
    #define AD8BITMODE TDC_FIELD(1,10)//[10:10]
    #define AD4BITMODE TDC_FIELD(1,9)//[9:9]
    #define DUMP_LCNTSEL TDC_FIELD(9,0)//[8:0]
    
#define COMB_CTRL_05 (IO_COMB_BASE + 0x654)
    #define VCNT3D_M TDC_FIELD(7,25)//[31:25]
    #define VLEN3D_M TDC_FIELD(9,16)//[24:16]
	#define HSYNCST TDC_FIELD(5,11)//[15:11]
    #define WINSEL TDC_FIELD(1,10)//[10:10]
    #define HCNT3D_M TDC_FIELD(10,0)//[9:0]
    
#define COMB_CTRL_06 (IO_COMB_BASE + 0x658)
    #define C_START_DISTANCE TDC_FIELD(16,16)//[31:16]
    #define C_PALMODE TDC_FIELD(1,12)//[12:12]
    #define C_ENGEN TDC_FIELD(1,11)//[11:11]
    #define C_FIELDI_SEL TDC_FIELD(1,10)//[10:10]
    #define C_LCNT_TOTAL TDC_FIELD(10,0)//[9:0]
    
#define COMB_CTRL_07 (IO_COMB_BASE + 0x65C)
    #define C_COMB_VCOUNT_REACH TDC_FIELD(10,16)//[25:16]
    #define C_VS_LCNT_FALL_1 TDC_FIELD(8,8)//[15:8]
    #define C_VS_LCNT_RISE_1 TDC_FIELD(8,0)//[7:0]
    
#define COMB_CTRL_08 (IO_COMB_BASE + 0x660)
    #define TIMEGEN_WIN_OR_ENABLE TDC_FIELD(1,30)//[30:30]
    #define DIS_AUTO_ENGEN TDC_FIELD(1,29)//[29:29]
    #define CB_VFSEL TDC_FIELD(2,27)//[28:27]
    #define WIN_DISABLE TDC_FIELD(1,26)//[26:26]
    #define C_X_Y_DISABLE TDC_FIELD(1,25)//[25:25]
    #define C_ENGEN_V_LOCK_EN TDC_FIELD(1,24)//[24:24]
    #define C_VS_LCNT_FALL_2 TDC_FIELD(9,12)//[20:12]
    #define C_VS_LCNT_RISE_2 TDC_FIELD(9,0)//[8:0]
    
#define COMB_CTRL_09 (IO_COMB_BASE + 0x664)
    #define C_LINE_LENGTH TDC_FIELD(11,20)//[30:20]
    #define C_LINE_ACTIVE TDC_FIELD(10,8)//[17:8]
    #define C_SWITCH_FIELD_LCNT TDC_FIELD(8,0)//[7:0]
    
#define COMB_CTRL_0A (IO_COMB_BASE + 0x668)
    #define PCNTH2_2 TDC_FIELD(8,24)//[31:24]
    #define PCNTH2_1 TDC_FIELD(24,0)//[23:0]
    
#define COMB_CTRL_0B (IO_COMB_BASE + 0x66C)
    #define PCNTH1_2 TDC_FIELD(8,24)//[31:24]
    #define PCNTH1_1 TDC_FIELD(24,0)//[23:0]
    
#define COMB_CTRL_0C (IO_COMB_BASE + 0x670)
    #define REG_DRAMENDADR TDC_FIELD(25,0)//[24:0]
    
#define COMB_CTRL_0D (IO_COMB_BASE + 0x674)
    #define REG_F2BLEN TDC_FIELD(4,28)//[31:28]
    #define REG_F1BLEN TDC_FIELD(4,24)//[27:24]
    #define REG_F0BLEN TDC_FIELD(4,20)//[23:20]
    #define CTRL_RESERVE1 TDC_FIELD(15,5)//[19:5]
    #define REG_GOHONLY TDC_FIELD(1,4)//[4:4]
    #define REG_DISGOHONLY TDC_FIELD(1,3)//[3:3]
    #define REG_WITHCLRTH_5 TDC_FIELD(1,2)//[2:2]
    #define REG_WITHCLRTH_4 TDC_FIELD(1,1)//[1:1]
    #define REG_DIRINSEL TDC_FIELD(1,0)//[0:0]
    
#define COMB_CTRL_0E (IO_COMB_BASE + 0x678)
    #define PERFIELDCVBSPHASESUM3 TDC_FIELD(8,24)//[31:24]
    #define PERFIELDCVBSPHASESUM2 TDC_FIELD(8,16)//[23:16]
    #define PERFIELDCVBSPHASESUM1 TDC_FIELD(8,8)//[15:8]
    #define PERFIELDCVBSPHASESUM0 TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_00 (IO_COMB_BASE + 0x67C)
    #define REG_CVSMTHS TDC_FIELD(8,24)//[31:24]
    #define REG_YSMTH TDC_FIELD(8,16)//[23:16]
    #define REG_OFFSETTH TDC_FIELD(8,8)//[15:8]
    #define REG_CVSMTH TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_01 (IO_COMB_BASE + 0x680)
    #define REG_DIS_HOR_SM TDC_FIELD(1,31)//[31:31]
    #define REG_EN_TMB_4PV TDC_FIELD(1,30)//[30:30]
    #define REG_DISMO4PV TDC_FIELD(1,29)//[29:29]
    #define DIS_BACKGROUND_SAME TDC_FIELD(1,28)//[28:28]
    #define REG_V_EDGE_TH TDC_FIELD(4,24)//[27:24]
    #define REG_C_DIFF_TH_4PV TDC_FIELD(8,16)//[23:16]
    #define REG_MBTH TDC_FIELD(8,8)//[15:8]
    #define REG_HFTH TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_02 (IO_COMB_BASE + 0x684)
    #define REG_ENPALDRAM TDC_FIELD(1,30)//[30:30]
    #define REG_ENSWDET TDC_FIELD(1,29)//[29:29]
    #define REG_MBTHSEL TDC_FIELD(1,28)//[28:28]
    #define REG_ENTMB TDC_FIELD(1,27)//[27:27]
    #define REG_ONE_LINE_MOTION_EN TDC_FIELD(1,26)//[26:26]
    #define REG_CVBS_MOTION_EN TDC_FIELD(1,25)//[25:25]
    #define REG_GRAYVERSMSEL TDC_FIELD(1,24)//[24:24]
    #define REG_DISSWEEPDET TDC_FIELD(1,23)//[23:23]
    #define REG_DISSTILLHS TDC_FIELD(1,22)//[22:22]
    #define REG_ENHSTO3D TDC_FIELD(1,21)//[21:21]
    #define REG_EN2DPUREVER1 TDC_FIELD(1,20)//[20:20]
    #define REG_ENCVMO TDC_FIELD(1,19)//[19:19]
    #define REG_ENMBEXIST TDC_FIELD(1,18)//[18:18]
    #define REG_ENCVBSVERSMOOTH TDC_FIELD(1,17)//[17:17]
    #define REG_ENCVBSHORSMOOTH TDC_FIELD(1,16)//[16:16]
    #define REG_HORMBTH TDC_FIELD(5,8)//[12:8]
    #define REG_VERMBTH TDC_FIELD(5,0)//[4:0]
    
#define COMB3D_03 (IO_COMB_BASE + 0x688)
    #define REG_CVBS_DIFF_TH TDC_FIELD(8,24)//[31:24]
    #define REG_LUMA_DIFF_TH TDC_FIELD(8,16)//[23:16]
    #define ONE_LINE_VER_DIFF TDC_FIELD(8,8)//[15:8]
    #define ONE_LINE_HOR_TH TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_04 (IO_COMB_BASE + 0x68C)
    #define REG_PATCH_TMB_TH TDC_FIELD(8,24)//[31:24]
    #define REG_PATCH_COST_MAX TDC_FIELD(8,16)//[23:16]
    #define REG_PATCH_LUMATH TDC_FIELD(8,8)//[15:8]
    #define REG_PATCH_COST_OFFSET TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_05 (IO_COMB_BASE + 0x690)
    #define RESERVE3D_1 TDC_FIELD(8,24)//[31:24]
    #define REG_EN_PATCH_STAIR TDC_FIELD(1,16)//[16:16]
    #define REG_MATRIX_EN TDC_FIELD(1,15)//[15:15]
    #define REG_RAMP1_EN TDC_FIELD(1,14)//[14:14]
    #define REG_RAMP_EN TDC_FIELD(1,13)//[13:13]
    #define REG_EN_ANT_PATCH TDC_FIELD(1,12)//[12:12]
    #define REG_LINEAR_VAR_2D_TH TDC_FIELD(4,8)//[11:8]
    #define REG_LINEAR_VAR_3D_TH TDC_FIELD(4,4)//[7:4]
    #define REG_LINEAR_VAR_2D_TH2 TDC_FIELD(4,0)//[3:0]
    
#define COMB3D_06 (IO_COMB_BASE + 0x694)
    #define REG_UNIFORMTH TDC_FIELD(8,24)//[31:24]
    #define REG_UNIAMPSM TDC_FIELD(8,16)//[23:16]
    #define REG_LOSTI_LUMATH TDC_FIELD(8,8)//[15:8]	
	#define REG_LOSTI_CVBSTH TDC_FIELD(8,0)//[7:0]
	
#define COMB3D_07 (IO_COMB_BASE + 0x698)
    #define REG_MOTION9X3TH TDC_FIELD(8,16)//[23:16]
    #define REG_MOTION9X3TH_B TDC_FIELD(8,8)//[15:8]
    #define REG_D3GAIN_Y_LS TDC_FIELD(4,4)//[7:4]
    #define REG_D3GAIN_CV_LS TDC_FIELD(4,0)//[3:0]
    
#define COMB3D_08 (IO_COMB_BASE + 0x69C)
	#define REG_WHIVPHTH TDC_FIELD(4,28)//[31:28]
	#define REG_HORCVBS_MAX_EN TDC_FIELD(1,27)//[27:27]
	#define REG_PENALTY2D_HV_MAX_EN TDC_FIELD(1,26)//[26:26]
	#define REG_SMALLONSEL TDC_FIELD(1,25)//[25:25]
    #define REG_ENPALCCS_NEW TDC_FIELD(1,24)//[24:24]
    #define REG_UNIFORMTH4CCS TDC_FIELD(8,16)//[23:16]
    #define REG_BETTERTH TDC_FIELD(8,8)//[15:8]
    #define REG_D2SMALLTH TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_09 (IO_COMB_BASE + 0x6A0)
    #define REG_3D_FW_COST TDC_FIELD(8,24)//[31:24]
    #define REG_2D_FW_COST TDC_FIELD(8,16)//[23:16]
    #define DIS_WRONG_EDGE TDC_FIELD(1,15)//[15:15]
    #define WRONG_EDGE_TH TDC_FIELD(3,12)//[14:12]
    #define REG_D2VERCVBSSEL TDC_FIELD(1,10)//[10:10]
    #define REG_PAL2DVERCVBSSEL TDC_FIELD(2,8)//[9:8]
    #define REG_PENALTY2D_MAX TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_0A (IO_COMB_BASE + 0x6A4)
    #define EN_TWO_LINE_MAX TDC_FIELD(1,29)//[29:29]
    #define REG_ENLOCALSTILLG_B TDC_FIELD(1,28)//[28:28]
    #define REG_ENLOCALSTILLG TDC_FIELD(1,27)//[27:27]
    #define REG_EN_SCENECHANGE TDC_FIELD(1,26)//[26:26]
    #define REG_WEIGHTLPFSEL TDC_FIELD(2,24)//[25:24]
    #define REG_D3BIGTH1 TDC_FIELD(8,16)//[23:16]
    #define REG_MOTION9X3TH_LS_NTSC TDC_FIELD(8,8)//[15:8]
    #define REG_LINEAR_VAR_OFFSET TDC_FIELD(4,4)//[7:4]
    #define REG_LINEAR_VAR_MAX TDC_FIELD(4,0)//[3:0]
    
#define COMB3D_0B (IO_COMB_BASE + 0x6A8)
    #define REG_D3BIGTH_SW TDC_FIELD(8,24)//[31:24]
    #define REG_D2D3SMALLTH TDC_FIELD(7,16)//[22:16]
    #define REG_INPHASE_NTSC_EN TDC_FIELD(1,15)//[15:15]
    #define REG_ENSWEEPGDET TDC_FIELD(1,14)//[14:14]
    #define REG_D3BIGTH_MIN TDC_FIELD(2,12)//[13:12]
    #define REG_D3BIGTH TDC_FIELD(10,0)//[9:0]
    
#define COMB3D_0C (IO_COMB_BASE + 0x6AC)
    #define CHROMA3D_OFFSET TDC_FIELD(8,24)//[31:24]
    #define CHROMA3D_SLOP TDC_FIELD(8,16)//[23:16]
    #define LUMAHOR_OFFSET TDC_FIELD(8,8)//[15:8]
    #define LUMAHOR_SLOP TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_0D (IO_COMB_BASE + 0x6B0)
    #define REG_MOTH4MB TDC_FIELD(8,24)//[31:24]
    #define REG_PERIOD_TH TDC_FIELD(8,16)//[23:16]
    #define DRAM_FRAME_CNT1 TDC_FIELD(3,8)//[10:8]
    #define DRAM_FRAME_CNT2 TDC_FIELD(3,4)//[6:4]
    #define DRAM_FRAME_CNT3 TDC_FIELD(3,0)//[2:0]
    
#define COMB3D_0E (IO_COMB_BASE + 0x6B4)
    #define REG_PERIOD_STEP TDC_FIELD(8,24)//[31:24]
    #define SP_VAR_MAX_RANGE1 TDC_FIELD(8,16)//[23:16]
    #define SP_VAR_MAX_RANGE2 TDC_FIELD(8,8)//[15:8]
    #define SP_VAR_TH TDC_FIELD(8,0)//[7:0]
	
//Page COMB_2
#define COMB3D_0F (IO_COMB_BASE + 0x6B8)
    #define REG_D2GAINCV_MB TDC_FIELD(4,24)//[27:24]
    #define REG_D3GAINY TDC_FIELD(4,20)//[23:20]
	#define REG_D3GAINC TDC_FIELD(4,16)//[19:16]
    #define REG_D3GAINCV TDC_FIELD(4,12)//[15:12]
	#define REG_D2GAINY TDC_FIELD(4,8)//[11:8]
    #define REG_D2GAINC TDC_FIELD(4,4)//[7:4]
    #define REG_D2GAINCV TDC_FIELD(4,0)//[3:0]
    
#define COMB3D_10 (IO_COMB_BASE + 0x6BC)
    #define REG_WHSWEEPEDGETH TDC_FIELD(8,24)//[31:24]
    #define REG_D3GAIN_Y_SW TDC_FIELD(4,20)//[23:20]
    #define REG_D3GAIN_C_SW TDC_FIELD(4,16)//[19:16]
    #define REG_D3GAIN_CV_SW TDC_FIELD(4,12)//[15:12]
    #define REG_D2GAIN_Y_SW TDC_FIELD(4,8)//[11:8]
    #define REG_D2GAIN_C_SW TDC_FIELD(4,4)//[7:4]
    #define REG_D2GAIN_CV_SW TDC_FIELD(4,0)//[3:0]
    
#define COMB3D_11 (IO_COMB_BASE + 0x6C0)
	#define REG_YDIFTH TDC_FIELD(8,20)//[27:20]
    #define REG_D3GAIN_Y_SC TDC_FIELD(4,16)//[19:16]
    #define REG_D3GAIN_C_SC TDC_FIELD(4,12)//[15:12]
    #define REG_D3GAIN_CV_SC TDC_FIELD(4,8)//[11:8]
    #define FLASH_FRM_MAX TDC_FIELD(4,4)//[7:4]
    #define FLASH_FRM TDC_FIELD(4,0)//[3:0]
    
#define COMB3D_12 (IO_COMB_BASE + 0x6C4)
    #define FLASH_NEI_TH TDC_FIELD(8,24)//[31:24]
    #define FLASH_MAX_TH TDC_FIELD(8,16)//[23:16]
    #define FLASH_AVG_TH TDC_FIELD(8,8)//[15:8]
    #define FLASH_PIX_TH TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_13 (IO_COMB_BASE + 0x6C8)
	#define REG_DZSM2DTH TDC_FIELD(8,20)//[27:20]
    #define D2D3SMALL_PRIORITY TDC_FIELD(1,19)//[19:19]
    #define REG_WDATASEL TDC_FIELD(2,16)//[17:16]
    #define REG_MOTIONTH TDC_FIELD(8,8)//[15:8]
	#define REG_CHECKFTH TDC_FIELD(4,4)//[7:4]
    #define REG_CHECKMAXF TDC_FIELD(4,0)//[3:0]
    
#define COMB3D_14 (IO_COMB_BASE + 0x6CC)
    #define REG_DARKG3 TDC_FIELD(4,28)//[31:28]
    #define REG_DARKG2 TDC_FIELD(4,24)//[27:24]
    #define REG_DARKTH TDC_FIELD(8,16)//[23:16]
    #define REG_ENYDIF TDC_FIELD(1,15)//[15:15]
    #define REG_ENGAINSTL TDC_FIELD(1,14)//[14:14]
    #define REG_ENDARKG TDC_FIELD(1,13)//[13:13]
    #define REG_ENGAINSML TDC_FIELD(1,12)//[12:12]
    #define REG_ENSWC3DSM TDC_FIELD(1,11)//[11:11]
    #define REG_CCDTH TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_15 (IO_COMB_BASE + 0x6D0)
    #define REG_GAIN3DST TDC_FIELD(4,28)//[31:28]
    #define REG_GAIN2DST TDC_FIELD(4,24)//[27:24]
    #define REG_GAIN3D_C3DSM TDC_FIELD(4,20)//[23:20]
    #define REG_GAIN2D_C3DSM TDC_FIELD(4,16)//[19:16]
    #define REG_GAIN3DL TDC_FIELD(4,12)//[15:12]
    #define REG_GAIN2DL TDC_FIELD(4,8)//[11:8]
    #define REG_GAIN3DN TDC_FIELD(4,4)//[7:4]
    #define REG_GAIN2DN TDC_FIELD(4,0)//[3:0]
    
#define COMB3D_16 (IO_COMB_BASE + 0x6D4)
    #define REG_WHLUMASUMTH TDC_FIELD(8,24)//[31:24]
    #define REG_ENWHDCDIF TDC_FIELD(1,19)//[19:19]
    #define REG_ENWHLUMA TDC_FIELD(1,15)//[15:15]
    #define REG_ENWHMBPIX TDC_FIELD(1,14)//[14:14]
    #define REG_ENWHCEDGE TDC_FIELD(1,13)//[13:13]
    #define REG_ENWHYDEPI TDC_FIELD(1,12)//[12:12]
    #define REG_ENWHEDGE TDC_FIELD(1,11)//[11:11]
    #define REG_ENWHCOLOR TDC_FIELD(1,10)//[10:10]
    #define REG_WHYDETH TDC_FIELD(10,0)//[9:0]
    
#define COMB3D_17 (IO_COMB_BASE + 0x6D8)
    #define REG_EN_FLASH_STATUS TDC_FIELD(1,29)//[29:29]
    #define REG_METRIC_INK TDC_FIELD(1,28)//[28:28]
    #define REG_START_M_INK_EN TDC_FIELD(1,25)//[25:25]
    #define REG_WHITEINK TDC_FIELD(1,24)//[24:24]
    #define REG_SCENE_CHANGE_SEQ_INK_EN TDC_FIELD(1,23)//[23:23]
    #define REG_INK_GREY_LEVEL TDC_FIELD(1,22)//[22:22]
    #define REG_SWININK TDC_FIELD(1,21)//[21:21]
    #define REG_WININK TDC_FIELD(1,20)//[20:20]
    #define REG_DRAM_REQ_INK TDC_FIELD(1,19)//[19:19]
    #define REG_PALSWEEPINK TDC_FIELD(1,18)//[18:18]
    #define REG_TOTALINK TDC_FIELD(6,12)//[17:12]
    #define REG_INKON TDC_FIELD(1,11)//[11:11]
    #define REG_WITHCLRTH_1 TDC_FIELD(1,10)//[10:10]
    #define REG_INKTH TDC_FIELD(10,0)//[9:0]
    
#define COMB3D_18 (IO_COMB_BASE + 0x6DC)
    #define REG_C3D_FIR_SEL TDC_FIELD(1,31)//[31:31]
    #define REG_EN_C3D_FIR TDC_FIELD(1,30)//[30:30]
    #define REG_ENFORCEC3D TDC_FIELD(1,23)//[23:23]
    #define REG_ENFORCEY3D TDC_FIELD(1,22)//[22:22]
    #define REG_ENFORCEY2D TDC_FIELD(1,21)//[21:21]
    #define REG_DISFIRINCBG TDC_FIELD(1,20)//[20:20]
    #define REG_ENADAPTCFIR TDC_FIELD(1,19)//[19:19]
    #define REG_ENFORCECFIR TDC_FIELD(1,18)//[18:18]
    #define REG_CFIRSEL TDC_FIELD(1,17)//[17:17]
    #define REG_DISCLUMAEDGE TDC_FIELD(1,16)//[16:16]
    #define REG_FIRMOTIONTH TDC_FIELD(8,8)//[15:8]
    #define REG_CHROMASUMBGTH TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_19 (IO_COMB_BASE + 0x6E0)
    #define REG_SENCECHANGETH TDC_FIELD(8,24)//[31:24]
    #define REG_WHSWEEPSUMTH TDC_FIELD(8,16)//[23:16]
    #define REG_MOTION_THBIG TDC_FIELD(8,8)//[15:8]
    #define REG_SMLMOTHC TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_1A (IO_COMB_BASE + 0x6E4)
    #define REG_NOISETH TDC_FIELD(8,24)//[31:24]
    #define REG_CZPTH TDC_FIELD(7,16)//[22:16]
    #define MANUAL_NOISE TDC_FIELD(1,13)//[13:13]
    #define MANUAL_NOISE_EN TDC_FIELD(1,12)//[12:12]
    #define REG_SMLCNTH TDC_FIELD(12,0)//[11:0]
    
#define COMB3D_1B (IO_COMB_BASE + 0x6E8)
    #define REG_DISSWEETDET TDC_FIELD(1,31)//[31:31]
    #define REG_ENSTFI TDC_FIELD(1,30)//[30:30]
    #define REG_NCSEL TDC_FIELD(1,29)//[29:29]
    #define REG_PALC5X3SEL TDC_FIELD(1,28)//[28:28]
    #define REG_PALSW2 TDC_FIELD(2,26)//[27:26]
    #define REG_PALSW1 TDC_FIELD(2,24)//[25:24]
    #define REG_SAMECSEL TDC_FIELD(1,23)//[23:23]
    #define REG_ENSAMEC TDC_FIELD(1,22)//[22:22]
    #define REG_DELAYSEL2 TDC_FIELD(2,20)//[21:20]
    #define REG_BLDCSEL TDC_FIELD(2,18)//[19:18]
    #define REG_UVSMSEL TDC_FIELD(2,16)//[17:16]
    #define REG_ENWHOLEVSMOOTH TDC_FIELD(1,15)//[15:15]
    #define REG_ENBSWDETPALHIGH TDC_FIELD(1,14)//[14:14]
    #define REG_ENBSWDETPAL TDC_FIELD(1,13)//[13:13]
    #define REG_ENSWDETPAL TDC_FIELD(1,12)//[12:12]
    #define REG_BSWDETPIXPAL TDC_FIELD(4,8)//[11:8]
    #define REG_BSWDETTHPAL TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_1C (IO_COMB_BASE + 0x6EC)
    #define RESERVE3D_2 TDC_FIELD(24,8)//[31:8]// cheng
    #define REG_DELAYSEL TDC_FIELD(2,6)//[7:6]
    #define CB_3D_MOTION_TH TDC_FIELD(6,0)//[5:0]
    
#define COMB3D_1D (IO_COMB_BASE + 0x6F0)
    #define CB_3D_RA_MOTION_LEVEL_INI TDC_FIELD(4,28)//[31:28]
    #define CB_3D_RA_LOW_CONTRAST_TH TDC_FIELD(3,24)//[26:24]
    #define CB_3D_RA_IIR_FACTOR_DN TDC_FIELD(4,20)//[23:20]
    #define CB_3D_RA_IIR_FACTOR_UP TDC_FIELD(4,16)//[19:16]
    #define CB_3D_SP_VAR_MIN TDC_FIELD(4,12)//[15:12]
    #define CB_3D_SP_VAR_STEP TDC_FIELD(4,8)//[11:8]
    #define CB_3D_SP_VAR_MAX TDC_FIELD(7,0)//[6:0]
    
#define COMB3D_1E (IO_COMB_BASE + 0x6F4)
    #define CB_3D_SP_VAR_W_MIN TDC_FIELD(4,24)//[27:24]
    #define CB_3D_SP_VAR_TH TDC_FIELD(4,20)//[23:20]
    #define CB_3D_UV_VAR_TH TDC_FIELD(4,16)//[19:16]
    #define CB_3D_SMALL_OBJECT_TH TDC_FIELD(4,12)//[15:12]
    #define CB_3D_MOTION_W_MIN TDC_FIELD(4,8)//[11:8]
    #define CB_3D_FIELD_MOTION_PIXEL_CNT TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_1F (IO_COMB_BASE + 0x6F8)
    #define CB_3D_CUBE_FILTER_Y_EN TDC_FIELD(1,31)//[31:31]
	#define CB_3D_CUBE_FILTER_C_EN TDC_FIELD(1,30)//[30:30]
    #define CB_3D_FIX_CUBE_WEIGHT_Y TDC_FIELD(1,29)//[29:29]
    #define CB_3D_FIX_CUBE_WEIGHT_C TDC_FIELD(1,28)//[28:28]
    #define CB_3D_FORCE_CUBE_WEIGHT_Y TDC_FIELD(4,24)//[27:24]
    #define CB_3D_FORCE_CUBE_WEIGHT_C TDC_FIELD(4,20)//[23:20]
    #define CB_3D_REGION_MOTION_TH TDC_FIELD(6,8)//[13:8]
    #define CB_3D_MM_MIN TDC_FIELD(3,5)//[7:5]
    #define CB_3D_MM_TH TDC_FIELD(5,0)//[4:0]
    
#define COMB3D_20 (IO_COMB_BASE + 0x6FC)
    #define CB_3D_MOVE_CONF_SEL TDC_FIELD(1,30)//[30:30]
    #define CB_3D_AB_BC_BLEND_RATIO TDC_FIELD(6,24)//[29:24]
    #define CB_3D_AB_BC_BLEND_MIN_TH TDC_FIELD(7,16)//[22:16]
    #define CB_3D_MOVE_CONF_RATIO TDC_FIELD(6,8)//[13:8]
    #define CB_3D_MOVE_CONF_MIN_TH TDC_FIELD(7,0)//[6:0]
    
#define COMB3D_21 (IO_COMB_BASE + 0x700)
    #define FW_RESERVE TDC_FIELD(32,0)//[31:0]
    
#define COMB3D_22 (IO_COMB_BASE + 0x704)
    #define CB_3D_C_FLICKER_FIELD_BASED_CNT TDC_FIELD(8,24)//[31:24]
    #define CB_3D_C_FLICKER_FIELD_CNT_HW_EN TDC_FIELD(1,23)//[23:23]
    #define CB_3D_C_FLICKER_DIS_AB_PHASE TDC_FIELD(1,22)//[22:22]
    #define CB_3D_C_FLICKER_FIELD_CNT_FW TDC_FIELD(4,16)//[19:16]
    #define CB_3D_C_FLICKER_FIELD_CNT_TH TDC_FIELD(4,12)//[15:12]
    #define CB_3D_C_FLICKER_CNT_TH TDC_FIELD(4,8)//[11:8]
    #define CB_3D_C_FLICKER_COST_2D TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_23 (IO_COMB_BASE + 0x708)
    #define CB_3D_SCENE_CHANGE_FOUND TDC_FIELD(1,31)//[31:31]
    #define CB_3D_SCENE_CHANGE_HW_EN TDC_FIELD(1,30)//[30:30]
    #define CB_3D_SCENE_CHANGE_INI TDC_FIELD(3,24)//[26:24]
    #define CB_3D_HW_SCENE_CHANGE_TH TDC_FIELD(6,16)//[21:16]
    #define CB_3D_C_FLICKER_C_TH TDC_FIELD(8,8)//[15:8]
    #define CB_3D_C_FLICKER_STATUS_C_TH TDC_FIELD(8,0)//[7:0]
    
#define COMB3D_24 (IO_COMB_BASE + 0x70C)
    #define CB_3D_START_M_HW_EN TDC_FIELD(1,16)//[16:16]
    #define CB_3D_START_M_FW TDC_FIELD(1,15)//[15:15]
    #define CB_3D_START_M_W_MIN TDC_FIELD(3,12)//[14:12]
    #define CB_3D_START_M_TH TDC_FIELD(4,8)//[11:8]
    #define CB_3D_START_M_CNT_TH2 TDC_FIELD(4,4)//[7:4]
    #define CB_3D_START_M_CNT_TH1 TDC_FIELD(4,0)//[3:0]
    
#define COMB3D_25 (IO_COMB_BASE + 0x710)
    #define RESERVE3D_3 TDC_FIELD(32,0)//[31:0]

#define COMB2D_00 (IO_COMB_BASE + 0x714)
    #define REG_ENMHFY TDC_FIELD(1,31)//[31:31]
    #define ENSVNO TDC_FIELD(1,30)//[30:30]
    #define VEXP_C TDC_FIELD(1,29)//[29:29]
    #define VEXP TDC_FIELD(1,28)//[28:28]
    #define ENFVY TDC_FIELD(1,27)//[27:27]
    #define ENFSVNO TDC_FIELD(1,26)//[26:26]
    #define EN_Y5X3OUT TDC_FIELD(1,25)//[25:25]
    #define Y2D_CSHAP_EN TDC_FIELD(1,24)//[24:24]
    #define ENMK4 TDC_FIELD(1,23)//[23:23]
    #define ENMK3 TDC_FIELD(1,22)//[22:22]
    #define ENMK2 TDC_FIELD(1,21)//[21:21]
    #define ENMK1 TDC_FIELD(1,20)//[20:20]
    #define ENFWEAKC TDC_FIELD(1,19)//[19:19]
    #define FORCEWEAKC TDC_FIELD(1,18)//[18:18]
    #define REG_PURE1D TDC_FIELD(1,17)//[17:17]
    #define CUSE5X3 TDC_FIELD(1,16)//[16:16]
    #define ENMBNEW_C TDC_FIELD(1,15)//[15:15]
    #define ENMBNEW_Y TDC_FIELD(1,14)//[14:14]
    #define ENSHARP TDC_FIELD(1,13)//[13:13]
    #define ENPUREHORMB TDC_FIELD(1,12)//[12:12]
    #define ENPUREHORXX TDC_FIELD(1,11)//[11:11]
    #define ENPALVCD TDC_FIELD(1,10)//[10:10]
    #define ENPALVCD2 TDC_FIELD(1,9)//[9:9]
    #define ENFIXVCD TDC_FIELD(1,8)//[8:8]
    #define ENFIXVCD2 TDC_FIELD(1,7)//[7:7]
    #define ENUNIFORM TDC_FIELD(1,6)//[6:6]
    #define ENSGDIRIN TDC_FIELD(1,5)//[5:5]
    #define INVSGDIRIN TDC_FIELD(1,4)//[4:4]
    #define ENINVDIADIR TDC_FIELD(1,3)//[3:3]
    #define ENFIROK TDC_FIELD(1,2)//[2:2]
    #define FORCEFIR TDC_FIELD(1,1)//[1:1]
    
#define COMB2D_01 (IO_COMB_BASE + 0x718)
    #define ENGOH TDC_FIELD(1,31)//[31:31]
    #define ENVPERS TDC_FIELD(1,30)//[30:30]
    #define COMB_ATPG_CT TDC_FIELD(1,29)//[29:29]
    #define COMB_ATPG_OB TDC_FIELD(1,28)//[28:28]
    #define MBBOUNDSEL TDC_FIELD(1,27)//[27:27]
    #define DISCFIRINCBG TDC_FIELD(1,26)//[26:26]
    #define FIRINSEL TDC_FIELD(1,25)//[25:25]
    #define DISCVBS01234SM TDC_FIELD(1,24)//[24:24]
    #define ENDIRIN TDC_FIELD(1,23)//[23:23]
    #define MBLSEL TDC_FIELD(1,22)//[22:22]
    #define MB51 TDC_FIELD(1,21)//[21:21]
    #define HFYTBSEL TDC_FIELD(1,20)//[20:20]
    #define HFYTBSEL2 TDC_FIELD(1,19)//[19:19]
    #define MBBOUNDARY_Y TDC_FIELD(1,18)//[18:18]
    #define MBBOUNDARY_C TDC_FIELD(1,17)//[17:17]
    #define PUREVERPAL TDC_FIELD(1,16)//[16:16]
    #define EN_VERYLP TDC_FIELD(1,15)//[15:15]
    #define DHYSEL TDC_FIELD(1,14)//[14:14]
    #define REG_ENSPC TDC_FIELD(1,13)//[13:13]
    #define REG_ENSPCPT TDC_FIELD(1,12)//[12:12]
    #define RESERVE2D_1 TDC_FIELD(12,0)//[11:0]
    
#define COMB2D_02 (IO_COMB_BASE + 0x71C)
    #define REG_MHFYK TDC_FIELD(4,24)//[27:24]
    #define REG_VDGYHORSMOOTHTH TDC_FIELD(8,16)//[23:16]
    #define DYLOMIN_THR TDC_FIELD(8,8)//[15:8]
    #define SVNOCGTH TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_03 (IO_COMB_BASE + 0x720)
    #define MK4 TDC_FIELD(4,28)//[31:28]
    #define MK3 TDC_FIELD(4,24)//[27:24]
    #define MK2 TDC_FIELD(4,20)//[23:20]
    #define MK1 TDC_FIELD(4,16)//[19:16]
    #define MATRIX_TH TDC_FIELD(8,8)//[15:8]
    #define SHARP_TBTH TDC_FIELD(7,0)//[6:0]
    
#define COMB2D_04 (IO_COMB_BASE + 0x724)
    #define CAMPGTH TDC_FIELD(8,24)//[31:24]
    #define FIRVERDIFTH TDC_FIELD(8,16)//[23:16]
    #define PALSW TDC_FIELD(2,8)//[9:8]
    #define GOHTH TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_05 (IO_COMB_BASE + 0x728)
    #define INKTH2D TDC_FIELD(8,24)//[31:24]
    #define VCD_TBTH TDC_FIELD(7,17)//[23:17]
    #define TBTH_CVAR TDC_FIELD(7,10)//[16:10]
    #define HORMBTH2D TDC_FIELD(5,5)//[9:5]
    #define VERMBTH2D TDC_FIELD(5,0)//[4:0]
	
//Page COMB_3
#define COMB2D_06 (IO_COMB_BASE + 0x72C)
    #define MBTH TDC_FIELD(8,24)//[31:24]
    #define REG_LSMOOTH TDC_FIELD(8,16)//[23:16]
    #define PVCVBSVERTH TDC_FIELD(8,8)//[15:8]
    #define HDVDTH TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_07 (IO_COMB_BASE + 0x730)
    #define HFY_YTH2 TDC_FIELD(7,24)//[30:24]
    #define VWY2CHROMATH TDC_FIELD(8,16)//[23:16]
    #define VWY2CHROMA_STEP TDC_FIELD(4,12)//[15:12]
    #define REG_SPC_HALFTH TDC_FIELD(4,8)//[11:8]
    #define REG_SPC_SUMTH TDC_FIELD(4,4)//[7:4]
    #define REG_ENUNIFORM_SPCLR TDC_FIELD(1,3)//[3:3]
    #define REG_ENBOUND TDC_FIELD(1,2)//[2:2]
    #define REG_BOUNDLEVELSEL TDC_FIELD(2,0)//[1:0]
    
#define COMB2D_08 (IO_COMB_BASE + 0x734)
    #define REG_HFY_VYD_GAIN TDC_FIELD(4,28)//[31:28]
    #define REG_HFY_VCD_GAIN TDC_FIELD(4,24)//[27:24]
    #define REG_HFY_HYD_GAIN TDC_FIELD(4,20)//[23:20]
    #define REG_HFY_HCD_GAIN TDC_FIELD(4,16)//[19:16]
    #define REG_HFY_MBVTH TDC_FIELD(8,8)//[15:8]
    #define REG_HFY_HDGAIN TDC_FIELD(4,4)//[7:4]
    #define CTPSEL TDC_FIELD(2,2)//[3:2]
    #define PVSEL TDC_FIELD(2,0)//[1:0]
    
#define COMB2D_09 (IO_COMB_BASE + 0x738)
    #define VYD_GAIN_HFY2 TDC_FIELD(4,28)//[31:28]
    #define VCD_GAIN_HFY2 TDC_FIELD(4,24)//[27:24]
    #define HYD_GAIN_HFY2 TDC_FIELD(4,20)//[23:20]
    #define HCD_GAIN_HFY2 TDC_FIELD(4,16)//[19:16]
    #define MBVTH_HFY2 TDC_FIELD(8,8)//[15:8]
    #define HDGAIN_HFY2 TDC_FIELD(4,4)//[7:4]
    #define CSEL TDC_FIELD(2,2)//[3:2]
    #define YSEL TDC_FIELD(2,0)//[1:0]
    
#define COMB2D_0A (IO_COMB_BASE + 0x73C)
    #define K2H4MB TDC_FIELD(4,28)//[31:28]
    #define K2HSEL TDC_FIELD(2,26)//[27:26]
    #define TBTH_K2B TDC_FIELD(7,16)//[22:16]
    #define K2HVERTH TDC_FIELD(8,8)//[15:8]
    #define VIPTH TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_0B (IO_COMB_BASE + 0x740)
    #define K2_TB7 TDC_FIELD(4,28)//[31:28]
    #define K2_TB6 TDC_FIELD(4,24)//[27:24]
    #define K2_TB5 TDC_FIELD(4,20)//[23:20]
    #define K2_TB4 TDC_FIELD(4,16)//[19:16]
    #define K2_TB3 TDC_FIELD(4,12)//[15:12]
    #define K2_TB2 TDC_FIELD(4,8)//[11:8]
    #define K2_TB1 TDC_FIELD(4,4)//[7:4]
    #define K2_TB0 TDC_FIELD(4,0)//[3:0]
    
#define COMB2D_0C (IO_COMB_BASE + 0x744)
    #define VCD_TB7 TDC_FIELD(4,28)//[31:28]
    #define VCD_TB6 TDC_FIELD(4,24)//[27:24]
    #define VCD_TB5 TDC_FIELD(4,20)//[23:20]
    #define VCD_TB4 TDC_FIELD(4,16)//[19:16]
    #define VCD_TB3 TDC_FIELD(4,12)//[15:12]
    #define VCD_TB2 TDC_FIELD(4,8)//[11:8]
    #define VCD_TB1 TDC_FIELD(4,4)//[7:4]
    #define VCD_TB0 TDC_FIELD(4,0)//[3:0]
    
#define COMB2D_0D (IO_COMB_BASE + 0x748)
    #define CHANNEL_B_SEL TDC_FIELD(1,29)//[29:29]
	#define DRAMBASEADR_MSB TDC_FIELD(1,28)//[28:28]
	#define CON4_B4 TDC_FIELD(1,27)//[27:27]
    #define CON4_B1 TDC_FIELD(1,26)//[26:26]
    #define CON4_B2 TDC_FIELD(1,25)//[25:25]
    #define CON4_B3 TDC_FIELD(1,24)//[24:24]
    #define U1_TH TDC_FIELD(8,16)//[23:16]
    #define REG_SPC_UVTH TDC_FIELD(8,8)//[15:8]
    #define CLGTH TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_0E (IO_COMB_BASE + 0x74C)
    #define INKSEL TDC_FIELD(1,30)//[30:30]
    #define COMB2D_REG_INK TDC_FIELD(6,24)//[29:24]
    #define TBTH_K2 TDC_FIELD(7,16)//[22:16]
    #define TBTH_K1V TDC_FIELD(7,8)//[14:8]
    #define TBTH_K1H TDC_FIELD(7,0)//[6:0]
    
#define COMB2D_0F (IO_COMB_BASE + 0x750)
    #define UV_DELAYSEL TDC_FIELD(2,30)//[31:30]
    #define Y2D_COLOR_HSLOPE TDC_FIELD(5,25)//[29:25]
    #define Y2D_COLOR_HUE_RANGE TDC_FIELD(6,19)//[24:19]
    #define Y2D_COLOR_SSLOPE TDC_FIELD(3,16)//[18:16]
    #define Y2D_COLOR_SAT_RANGE TDC_FIELD(8,8)//[15:8]
    #define Y2D_COLOR_SLOPE TDC_FIELD(2,5)//[6:5]
    #define Y2D_COLOR_SIMILAR_TH TDC_FIELD(5,0)//[4:0]
    
#define COMB2D_10 (IO_COMB_BASE + 0x754)
    #define Y2D_BAND23FILTER_SEL TDC_FIELD(1,31)//[31:31]
    #define Y2D_COLOR_UVSLOPE TDC_FIELD(3,21)//[23:21]
    #define Y2D_COLOR_UVVAR_TH TDC_FIELD(6,15)//[20:15]
    #define Y2D_COLOR_GSLOPE TDC_FIELD(3,6)//[8:6]
    #define Y2D_COLOR_GRAY_RANGE TDC_FIELD(6,0)//[5:0]
    
#define COMB2D_11 (IO_COMB_BASE + 0x758)
    #define Y2D_CSHAP_K4_OFFSET TDC_FIELD(3,28)//[30:28]
    #define Y2D_CSHAP_K3_OFFSET TDC_FIELD(3,24)//[26:24]
    #define Y2D_CSHAP_K2_OFFSET TDC_FIELD(3,20)//[22:20]
    #define Y2D_CSHAP_K1_OFFSET TDC_FIELD(3,16)//[18:16]
    #define Y2D_CSHAP_K4_GAIN TDC_FIELD(4,12)//[15:12]
    #define Y2D_CSHAP_K3_GAIN TDC_FIELD(4,8)//[11:8]
    #define Y2D_CSHAP_K2_GAIN TDC_FIELD(4,4)//[7:4]
    #define Y2D_CSHAP_K1_GAIN TDC_FIELD(4,0)//[3:0]
    
#define COMB2D_12 (IO_COMB_BASE + 0x75C)
    #define RESERVE2D_2 TDC_FIELD(12,20)//[31:20]
    #define Y2D_CSHAP_EDGE_TH TDC_FIELD(6,14)//[19:14]
    #define Y2D_CSHAP_CORING TDC_FIELD(6,8)//[13:8]
    #define Y2D_CSHAP_CLIP_TH TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_13 (IO_COMB_BASE + 0x760)
    #define Y2D_CSHAP_DY_CLIP_EN TDC_FIELD(1,31)//[31:31]
    #define Y2D_CSHAP_DY_CLIP_GAIN TDC_FIELD(3,28)//[30:28]
    #define REG_SHARP_2D_CVAR_SEL TDC_FIELD(1,27)//[27:27]
    #define REG_CUBE_CVAR_SEL TDC_FIELD(1,26)//[26:26]
    #define REG_LEGACY_CVAR_SEL TDC_FIELD(1,25)//[25:25]
    #define DATA_POSITION_Y TDC_FIELD(12,12)//[23:12]
    #define DATA_POSITION_X TDC_FIELD(12,0)//[11:0]
    
#define COMB2D_14 (IO_COMB_BASE + 0x764)
    #define DUMP_DATA_SEL TDC_FIELD(2,22)//[23:22]
    #define DUMP_DATA_B TDC_FIELD(10,12)//[21:12]
    #define DUMP_DATA_A TDC_FIELD(10,0)//[9:0]
    
#define COMB2D_15 (IO_COMB_BASE + 0x768)
    #define C2D_COLOR_HUE_RANGE TDC_FIELD(6,24)//[29:24]
    #define C2D_COLOR_SAT_RANGE TDC_FIELD(8,16)//[23:16]
	#define C2D_COLOR_GSIMILAR_TH TDC_FIELD(5,8)//[12:8]
	#define C2D_COLOR_SIMILAR_TH TDC_FIELD(5,0)//[4:0]
	
#define COMB2D_16 (IO_COMB_BASE + 0x76C)
    #define C2D_COLOR_FORCE_GRAYWGTC TDC_FIELD(2,30)//[31:30]
	#define C2D_COLOR_FORCE_COLORWGTC TDC_FIELD(2,28)//[29:28]
	#define C2D_COLOR_SMIN_YDIFF TDC_FIELD(10,14)//[23:14]
    #define C2D_COLOR_UVVAR_TH TDC_FIELD(6,8)//[13:8]
    #define C2D_COLOR_GRAY_RANGE TDC_FIELD(6,0)//[5:0]

#define COMB2D_17 (IO_COMB_BASE + 0x770)
    #define C2D_COLOR_CBANDWGT_BSUMTH TDC_FIELD(8,24)//[31:24]
    #define C2D_COLOR_GBANDWGT_BSUMTH TDC_FIELD(8,16)//[23:16]
    #define C2D_COLOR_CBANDWGT_YRNGTH TDC_FIELD(8,8)//[15:8]
    #define C2D_COLOR_GBANDWGT_YRNGTH TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_18 (IO_COMB_BASE + 0x774)
    #define C2D_COLOR_GRNG_YRNG TDC_FIELD(8,24)//[31:24]
    #define C2D_COLOR_GRNG_BSUM TDC_FIELD(8,16)//[23:16]
	#define C2D_COLOR_MAXGRNG_YRNG TDC_FIELD(8,8)//[15:8]
    #define C2D_COLOR_MAXGRNG_BSUM TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_19 (IO_COMB_BASE + 0x778)
	#define Y2D_CBAND_CSEL TDC_FIELD(2,30)//[31:30]
	#define C2D_COLOR_CLINE_HRANGE TDC_FIELD(6,24)//[29:24]
    #define C2D_COLOR_CLINE_GRANGE TDC_FIELD(6,18)//[23:18]
    #define C2D_COLOR_GWGT_SATLIMIT_TYPE TDC_FIELD(2,16)//[17:16]
	#define C2D_COLOR_GWGT_YRNG TDC_FIELD(8,8)//[15:8]
    #define C2D_COLOR_GWGT_BSUM TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_1A (IO_COMB_BASE + 0x77C)
    #define C2D_CBAND_GRAY_RATIO TDC_FIELD(3,29)//[31:29]
	#define C2D_COLOR_CLINE_GDIFF TDC_FIELD(5,24)//[28:24]
    #define C2D_COLOR_CLINE_HSIMILAR TDC_FIELD(5,16)//[20:16]
    #define C2D_COLOR_CLINE_YRNG TDC_FIELD(8,8)//[15:8]
    #define C2D_COLOR_CLINE_BSUM TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_1B (IO_COMB_BASE + 0x780)
    #define C2D_CBAND_FORCE_CBWGT TDC_FIELD(2,30)//[31:30]
    #define C2D_CBAND_RVRATIO TDC_FIELD(5,24)//[28:24]
    #define C2D_CBAND_YRNG TDC_FIELD(8,16)//[23:16]
    #define C2D_CBAND_HRNG TDC_FIELD(8,8)//[15:8]
    #define C2D_CBAND_CDIFF TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_1C (IO_COMB_BASE + 0x784)
    #define Y2D_CBAND_BLENDWGT TDC_FIELD(4,28)//[31:28]
    #define Y2D_CBAND_MODE TDC_FIELD(2,24)//[25:24]
    #define C2D_CBAND_CVRATIO TDC_FIELD(5,16)//[20:16]
    #define C2D_CBAND_CVDIFF TDC_FIELD(8,8)//[15:8]
    #define C2D_CBAND_MAXCV TDC_FIELD(8,0)//[7:0]
    
#define COMB2D_1D (IO_COMB_BASE + 0x788)
    #define FRM_SAT_SUM TDC_FIELD(8,24)//[31:24]
    #define FRM_UVVAR_SUM TDC_FIELD(8,16)//[23:16]
    #define FRM_YBAND_VSHIFT TDC_FIELD(4,12)//[15:12]
    #define FRM_YBAND_HSHIFT TDC_FIELD(4,8)//[11:8]
    #define FRM_SAT_SUM_VSHIFT TDC_FIELD(4,4)//[7:4]
    #define FRM_SAT_SUM_HSHIFT TDC_FIELD(4,0)//[3:0]
    
#define COMB2D_1E (IO_COMB_BASE + 0x78C)
    #define FRM_YBAND1_SUM TDC_FIELD(8,24)//[31:24]
    #define FRM_YBAND2_SUM TDC_FIELD(8,16)//[23:16]
    #define FRM_YBAND3_SUM TDC_FIELD(8,8)//[15:8]
    #define FRM_YBAND4_SUM TDC_FIELD(8,0)//[7:0]

#define REG_SYS_0A (IO_COMB_BASE + 0x7D4)
	#define COMB3D_BIT27 TDC_FIELD(4,28)//[31:28]
/********************************************************/
#define IO_TVD_BASE        (IO_VBASE_VA+0x5b000)

#define CTG_07 (IO_TVD_BASE + 0x5fc)
   #define UV_DELAY    TDC_FIELD(2, 2) //3:2
   #define Y_DELAY	   TDC_FIELD(2, 0) //1:0
	
 #define VSRC_07 (IO_TVD_BASE + 0x434)
    #define AAF_SEL 	TDC_FIELD(3, 17) //19:17
    #define VSRC_SVID 	TDC_FIELD(1, 7) //7
		
 #define CTG_00 (IO_TVD_BASE + 0x5e0)
    #define BST_0DEG 	TDC_FIELD(1, 10) //10
		
#define STA_CDET_00 	(IO_TVD_BASE + 0x080)
	#define MODE_TVD3D		TDC_FIELD(3, 28) //30:28
	#define PHALT_TVD3D 	TDC_FIELD(1, 24) //24
	#define VPRES_TVD3D 	TDC_FIELD(1, 12) //12

#if defined(__ARM2__)
/*for arm2*/
#define TVD_VDOIN  (0xf00U)
/*#define TVD_VDOIN_ENABLE  (0x1 << 0)*/
/*#define TVD_VDOIN_DISABLE (0x0 << 0)*/
#elif defined(__UBOOT__)
/*for uboot*/
#else
/*for arm1*/
#endif



#if defined(__ARM2__)
#define TVD_BASE     0x10000000
#define IO_VBASE     0x10000000
#define IO_VBASE_VA  0x10000000
#define HAL_WRITE32(_reg_, _val_)    (*((volatile unsigned int*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)            (*((volatile unsigned int*)(_reg_)))
#else
#define TVD_BASE     0x0
#define IO_VBASE     IO_VBASE_VA

//#define HAL_WRITE32(_reg_, _val_)    __raw_writel((_val_), (_reg_))
//#define HAL_READ32(_reg_)            __raw_readl((_reg_))
#define HAL_WRITE32(_reg_, _val_)    (*((volatile unsigned int*)(_reg_)) = (_val_))
#define HAL_READ32(_reg_)            (*((volatile unsigned int*)(_reg_)))
#endif


#define IO_READ32(base, offset)         HAL_READ32((base) + (offset))
#define IO_WRITE32(base, offset, value) HAL_WRITE32((base) + (offset), (value))

#define TVD_ANA_READ32(offset)          IO_READ32(IO_VBASE, (offset))
#define TVD_ANA_WRITE32(offset, value)  IO_WRITE32(IO_VBASE, (offset), (value))

/* Register R/W define*/
#define TVD_READ32(offset)              IO_READ32(TVD_BASE, (offset))
#define TVD_WRITE32(offset, value)      IO_WRITE32(TVD_BASE, (offset), (value))

#define TVD_SET_BIT(offset, Bit)        TVD_WRITE32((offset), TVD_READ32((offset)) | (Bit))
#define TVD_CLR_BIT(offset, Bit)        TVD_WRITE32((offset), TVD_READ32((offset)) & (~(Bit)))

/* Mask 1 availed*/
#define TVD_WRITE32_MASK(offset, value, mask)    (TVD_WRITE32((offset), (((value) & (mask)) | (TVD_READ32((offset)) & (~mask)))))

/* snow mode */
#define SNOW_MODE 0x414U
#define SNOW_MODE_ON 0x00000200U /*8 bit set 0, 9 bit set 1*/
#define SNOW_MODE_ALAWAYS_ON 0x00000100U /*8 bit set 1*/

/* line_average */
#define LINE_AVERAGE 0x424U
#define LINE_AVERAGE_ON 0x0U
#define LINE_AVERAGE_OFF 0x10000000U
/* color_process */
#define COLOR_PROCESS_S0 0x418U
#define COLOR_PROCESS_S1 0x41CU
#define COLOR_PROCESS_S2 0x420U
#define COLOR_PROCESS_S3 0x424U
#define COLOR_PROCESS_S4 0x428U

#endif


