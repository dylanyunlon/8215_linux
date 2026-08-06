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
#ifndef _CP_REG_H_
#define _CP_REG_H_

#define CP_SUPPORT

#ifdef CP_SUPPORT
#include "cp_feature.h"

extern u32 _u4VCP_DBG_LVL;
extern u8 *_pcVcpLogLevel[];

#define VCP_LOG_LVL_OFF                         0
#define VCP_LOG_LVL_ERR                         1
#define VCP_LOG_LVL_WARN                        2
#define VCP_LOG_LVL_CLI                         3
#define VCP_LOG_LVL_INFO                        4
#define VCP_LOG_LVL_HAL                         5
#define VCP_LOG_LVL_IRQ                         6
#define VCP_LOG_LVL_TRACE                       7
#define VCP_LOG_LVL_DBG                         8
#define VCP_LOG_LVL_REGRW                       9

#ifdef __ARM2__
#define VCP_LOG(lvl, formatStr, ...)\
    do{ \
        if (lvl <= _u4VCP_DBG_LVL) {\
            printk("[ARM2 VCP]"formatStr, ##__VA_ARGS__);\
        }\
    } while (0)

#else
#define VCP_LOG(lvl, formatStr, ...)\
do { \
	if (lvl <= _u4VCP_DBG_LVL) {\
		if (lvl == VCP_LOG_LVL_ERR) {\
			pr_err("[VCP] %s: "formatStr, _pcVcpLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else if (lvl == VCP_LOG_LVL_WARN) {\
			pr_warn("[VCP] %s: "formatStr, _pcVcpLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else if (lvl == VCP_LOG_LVL_INFO) {\
			pr_info("[VCP] %s: "formatStr, _pcVcpLogLevel[lvl], ##__VA_ARGS__); \
		} \
/*		else if (lvl == VCP_LOG_LVL_DBG) {\
			pr_debug("%s: "formatStr, _pcVcpLogLevel[lvl], ##__VA_ARGS__); \
		} \
*/		else {\
			pr_debug("[VCP] %s: "formatStr, _pcVcpLogLevel[lvl], ##__VA_ARGS__); \
		} \
	} \
} while (0)
#endif


/* *********************************************************************/
/* ColorProcess Registers define				       */
/* *********************************************************************/
/* Panel Color Proess: 0x42600 */
#define RW_PCLRP_PTNGEN_L                   (0x00)
  #define MLC_PTN_ENA        (0x01<<20)
  #define MLC_PTN_STEP       (0x07<<24)
  #define MLC_PTN_PREC       (0x03<<28)
  #define MLC_PTN_ENA_SHF		(20)
  #define MLC_PTN_STEP_SHF	  (24)
  #define MLC_PTN_PREC_SHF	  (28)

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
  #define MLC_XFRONT         (0x01<<19) /* use to xor with input src Msb(so can chg Yc 2 Yuv) */
  #define MLC_PTN_ENA_U      (0x01<<20)
  #define MLC_PTN_STEP_U     (0x07<<21)
  #define MLC_PTN_PREC_U     (0x03<<24)
  #define MLC_PTN_ENA_V      (0x01<<26)
  #define MLC_PTN_STEP_V     (0x07<<27)
  #define MLC_PTN_PREC_V     (0x03<<30)

  #define MLC_XFRONT_SHF	 (19) /* use to xor with input src Msb(so can chg Yc 2 Yuv) */
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


#define RW_PCLRP_SATURATION                 (0x2C)
  #define SAT_GAIN           (0xFF)
  #define SAT_GAIN_SHF       (0)
  /* 0x00 (gain = 0) - 0x80 (gain = 1.0) - 0xFF (gain=2.0). Saturation gain = setting/0x80.*/

#define RW_PCLRP_HUE_SCECTRL                (0x30)
#define MLC_SCE_ENA        (0x01<<0)
#define SRAM_WR_MODE       (0x01<<1)
#define SRAM_RD_ENA        (0x01<<4)
#define HUE_GLOB_PREC      (0x01<<6)
#define HUE_DEGREE         (0x3F<<8)
#define SCE_UCLAMP         (0x7<<20)
#define SCE_VCLAMP         (0x7<<24)
#define MLC_SCE_ENA_SHF    (0)
#define SRAM_RD_ENA_SHF		(4)
#define HUE_GLOB_PREC_SHF  (6)
#define HUE_DEGREE_SHF     (8)
#define SCE_UCLAMP_SHF     (20)
#define SCE_VCLAMP_SHF     (24)

#define RW_PCLRP_HUE_SCE_CFG               (0x34)
  #define HUE_DG_PREC        (0x01<<0)

#define RW_PCLRP_SCE_TABLE                  (0x38)
  #define MLC_LD_ENA         (0x01<<0)

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

#define RW_UV2CBCR_CONV		(0x44)
#define MLC_V2CR_GAIN		(0x1FF<<20)
#define MLC_U2CB_GAIN		(0x1FF<<8)
#define MLC_UV_CONV_EN		(0x01<<0)
#define MLC_OUTFRONT		(0x01<<4)     /* signed /unsigned flag for  xor */
#define MLC_V2CR_GAIN_SHF	(20)
#define MLC_U2CB_GAIN_SHF	(8)
#define MLC_OUTFRONT_SHF	(4)
#define MLC_UV_CONV_EN_SHF	(0)

#define RW_PCLRP_BLACK_EXTENSION	(0x4c)
#define BLEND_ENA		(0x01<<31)
#define BLEV_EDIF		(0x01<<20)
#define BLEND_SLOPE		(0x3FF<<8)
#define BLEND_ANCHOR		(0x3F<<0)
#define BLEND_ENA_SHF		(31)
#define BLEV_EDIF_SHF		(20)
#define BLEND_SLOPE_SHF		(8)
#define BLEND_ANCHOR_SHF	(0)

#define RW_PCLRP_WHITE_EXTENSION	(0x50)
#define WLEND_ENA		(0x01<<31)
#define WLEV_EDIF		(0x01<<20)
#define WLEND_SLOPE		(0x7FF<<8)
#define WLEND_ANCHOR		(0x7F<<0)
#define WLEND_ENA_SHF		(31)
#define WLEV_EDIF_SHF		(20)
#define WLEND_SLOPE_SHF		(8)
#define WLEND_ANCHOR_SHF	(0)

#define RW_PCLRP_CTI		(0x54)
#define CTI_T_SELECT		(0x07<<0)
#define CTI_GAIN_SHARP	(0x7f<<4)
#define CTI_DZONE		(0xF<<12)
#define CTI_MAX_MIN_JG	(0x1<<16)
#define CTI_PTADDSUB_INV	(0x1<<17)
#define CTI_LP_ENA		(0x1<<20)
#define CTI_LP_SEL		(0x3<<21)
#define CTI_T_SELECT_SHF	(0)
#define CTI_GAIN_SHARP_SHF	(4)
#define CTI_DZONE_SHF	(12)
#define CTI_MAX_MIN_JG_SHF	(16)
#define CTI_PTADDSUB_INV_SHF	(17)
#define CTI_LP_ENA_SHF	(20)
#define CTI_LP_SEL_SHF	(21)

#define RW_PCLRP_CTI1                       (0x5c)
#define CTI_FIR_COEFF      (0xfffff<<8)
#define CTI_FIR_COEFF_SHF     (8)
#define RW_PCLRP_CTI2                       (0x60)
#define HD_AMP             (0xFF<<8)
#define HDDETECT_EN         (0x1<<16)
#define HD_AMP_SHF			(8)
#define HDDETECT_EN_SHF       (16)
#if defined(CP_FEATURE_SCE_TAB_READ)
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

typedef enum {
CLRP_SCE_TABLE,    /* sce data upt */
CLRP_SCE_ONOFF,    /* on off select */
CLRP_SCE_LUMATYPE, /* luma gain or add */
CLRP_SCE_TEST,     /* fix table */
CLRP_SCE_HUE_GLOBAL,
CLRP_SCE_SETTING_CFG,
CLRP_SCE_GET_TABLE,
CLRP_SCE_USER      /* user table */

} CP_SCE_CMD_T;

typedef enum {
CP_CONTR_GAIN,
CP_CONTR_BRIT,
CP_CONTR_SATR,
CP_CONTR_RESET,
CP_CONTR_INFLUM,/* not be used */
CP_CONTR_CORE   /* not be used */
} CP_CONTR_CMD_T;

typedef enum {
BLACK_LEVEL_EX,
WHITE_LEVEL_EX,
BW_LEVEL_EX_OFF
} CP_BW_EX_CMD_T;

typedef enum {
CLRP_GPP_DELAY,
CLRP_GPP_OFFSET1,
CLRP_GPP_OFFSET2,
CLRP_GPP_GAIN,
CLRP_GPP_INV,
CLRP_GPP_FIX,
CLRP_GPP_RST

} CP_GPP_PROC_CMD_T;


typedef enum {
CLRP_Y_CHANEL,
CLRP_U_CHANEL,
CLRP_V_CHANEL,
CLRP_ALL_CHANEL
} CLRP_YUV_CHAENL_SEL_T;

/* *********************************************************************/
/* ColorProcess(CP) Related const Table define			       */
/* *********************************************************************/

static const u32 _pdCPTestSCETable[360] =/*red param is modified. */
{
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

#endif /* CP_SUPPORT */

#endif /* _CP_REG_H_ */

