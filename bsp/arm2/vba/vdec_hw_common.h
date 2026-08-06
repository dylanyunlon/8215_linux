/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _VDEC_HW_COMMON_H_
#define _VDEC_HW_COMMON_H_

#include "vdec_usage.h"
#include "vdec_info_common.h"

#define HAL_HANDLE_OK 0x1

//#define VDEC_SIM_DUMP
#define VDEC_RDREG_TIMES 0x5000
//#define IO_BASE_ADDRESS                     0x70000000L
// *********************************************************************
// Video Decoder Registers define
// *********************************************************************
#define VLD_REG_OFFSET0   0x27000
#define VLD_TOP_REG_OFFSET0   (VLD_REG_OFFSET0 + 0x800)
#define MC_REG_OFFSET0    0x28000
#define AVC_VLD_REG_OFFSET0      0x29000
#define AVC_MV_REG_OFFSET0       0x2A000
#define AVC_FG_REG_OFFSET0        0x31000 // not used
#define PP_REG_OFFSET0          0x2c000
#define DV_REG_OFFSET0        0x2B000


#define VLD_REG_OFFSET1   0x2E000
#define MC_REG_OFFSET1    0x2F000
#define AVC_VLD_REG_OFFSET1      0x30000
#define AVC_MV_REG_OFFSET1       0x31000
#define AVC_FG_REG_OFFSET1        0x32000
#define DV_REG_OFFSET1        0x32000
#define PP_REG_OFFSET1          0x2c000
#define VLD_TOP_REG_OFFSET1   (VLD_REG_OFFSET1 + 0x800)

#define VDEC_MISC_BASE          DV_REG_OFFSET0


#define VDEC_CRC_REG_OFFSET0    0x2B000
#define VDEC_CRC_REG_EN              0x4
#define VDEC_CRC_EN                      0x1
#define VDEC_CRC_SRC_MC              (0x0 << 1)
#define VDEC_CRC_SRC_PP              (0x1 << 1)
#define VDEC_CRC_Y_CHKSUM0       0x8
#define VDEC_CRC_Y_CHKSUM1       0xC
#define VDEC_CRC_Y_CHKSUM2       0x10
#define VDEC_CRC_Y_CHKSUM3       0x14
#define VDEC_CRC_C_CHKSUM0       0x18
#define VDEC_CRC_C_CHKSUM1       0x1C
#define VDEC_CRC_C_CHKSUM2       0x20
#define VDEC_CRC_C_CHKSUM3       0x24


#define RW_PDN_CTRL                   0x0
#define RW_SYS_CLK_SEL                0x84
#define RW_PDN_CRTL_SPEC              0xC8
#define RW_PDN_CRTL_MODULE1           0xCC
#define RW_PDN_CRTL_MODULE2           0x178



#define VDEC_LOCAL_ARBITER 0x52084
    #define VDEC_LOCAL_ARBITER_RESET (0x1 << 12)
    #define VDEC_LOCAL_ARBITER_RESUM  0xFFFFEFFF

#define RW_VLD_TOP_ERROR_DETECT_TYPE (0x80001030)

#define RW_VLD_TOP_TIMEOUT_ST_CLR           0x04
   #define VLD_TOP_TIMEOUT_CLR          ((unsigned)0x01 << 0)
#define RW_VLD_OK_CLR   0x08 //From TV
#define RW_VLD_TOP_PRED_ADDR                0x28
#define RW_VLD_TOP_PRED_SRAM_CFG            0x2C // 11x4
   #define RW_PRED_SRAM_CFG_OUTRANGE_SUPPORT_FLAG   31
   #define RW_PRED_SRAM_CFG_SRAM_BYPASS_FLAG        30
#define RW_VLD_TOP_SEGID_DRAM_ADDR          0x3c // 15x4
#define RW_VLD_TOP_PRED_MODE                0x40 // 16x4
   #define RW_NUM_ROW_SEGID_DATA_M1_S   0
   #define RW_NUM_ROW_SEGID_DATA_M1_E   2
   #define RW_DRAM_BURST_MODE_S         4
   #define RW_DRAM_BURST_MODE_E         5
#define RW_VLD_TOP_WEBP_CTRL                0x48 // 18x4
   #define RW_BUFCTRL_ON_FLAG           0
   #define RW_RESIZE_INTER_ON_FLAG      4
#define RW_VLD_TOP_MB_DECSTART              0x4c // 19x4

#define RW_VLD_TOP_TIMEOUT_THD              0x50
   #define VLD_TOP_TIMEOUT_THD          0xffffffff

#define RW_VLD_TOP_TIMEOUT_SW               0x54
   #define VLD_TOP_DEC_CYCLE_EN         ((unsigned)0x1 << 0)

#define RW_VLD_TOP_BUSY_THRESHOLD           0x58
#define RW_VLD_TOP_ERR_CONCEAL              0x5C
   #define RW_RETURN_IF_ERROR_FLAG      31
   #define RW_ERROR_SWITCH_S            28
   #define RW_ERROR_SWITCH_E            29
   #define RW_SLICE_RECONCEL_MODE_S     8
   #define RW_SLICE_RECONCEL_MODE_E     9
   #define RW_SLICE_RECONCEL_FLAG       0

#define RW_VLD_TOP_ERR_TYPE_ENABLE          0x64
#define RW_VLD_TOP_PIC_MB_SIZE_M1           0x68  //RM PICSIZE2 PIC_WIDTH in mb & PIC_HEIGHT in mb
#define RW_VLD_TOP_PIC_PIX_SIZE             0x70  //RM PICSIZE1, PIC_WIDTH & PIC_HEIGHT
    #define RW_PIC_WIDTH_IN_MBS_S       0
    #define RW_PIC_WIDTH_IN_MBS_E       9
    #define RW_PIC_HEIGHT_IN_MBS_S      16
    #define RW_PIC_HEIGHT_IN_MBS_E      25
#define RW_VLD_TOP_TOTAL_MBS_IN_PIC         0x6C  //Total mbs in Pic RM PICSIZE3


// *********************************************************************
// VLD Registers define
// *********************************************************************
// Barral Shifter from 0x00 - 0x80
#define RO_VLD_BARL           0x00
#define RW_VLD_VDOUFM     0x84
    #define VLD_VDOUFM       ((unsigned)0x1 << 0)
    #define VLD_MXOFF         ((unsigned)0x1 << 8)
    #define VLD_ENSTCNT      ((unsigned)0x1 << 9)
    #define VLD_AATO           ((unsigned)0x1 << 10)

// Type 18 [BIT14] out_of_picture error
// Type 1  [BIT15] block pixel overflow
// Type 2  [BIT16] cbp number mismatch
// Type 3  [BIT17] dct_dc_size_lum table error
// Type 4  [BIT18] dct_dc_size_chrom table error
// Type 5  [BIT19] slice-layer syntax error
// Type 6  [BIT20] end_of_block syntax rooer
// Type 7  [BIT21] macroblock address increment table error
// Type 8  [BIT22] coded_block_pattern table error
// Type 9  [BIT23] insufficient slice error
// Type 10 [BIT24] slice-level quantization scale error
// Type 11 [BIT25] other slice-level syntax error
// Type 12 [BIT26] macroblock address overflow error
// Type 13 [BIT27] mb_type table error
// Type 14 [BIT28] motion type error
// Type 15 [BIT29] motion code error
// Type 16 [BIT30] marker syntax error
// Type 17 [BIT31] MC-IDCT busy overflow
  #define ERR_OUT_PIC                 ((unsigned)0x1 << 14)
  #define ERR_BL_PIX                   ((unsigned)0x1 << 15)
  #define ERR_CBP_NS                  ((unsigned)0x1 << 16)
  #define ERR_DCT_Y_TAB             ((unsigned)0x1 << 17)
  #define ERR_DCT_C_TAB             ((unsigned)0x1 << 18)
  #define ERR_SL_SYN                   ((unsigned)0x1 << 19)
  #define ERR_EOB_SYN                ((unsigned)0x1 << 20)
  #define ERR_MB_ADD_INC_TAB   ((unsigned)0x1 << 21)
  #define ERR_CBP_TAB                ((unsigned)0x1 << 22)
  #define ERR_INS_SL                   ((unsigned)0x1 << 23)
  #define ERR_SL_QSCALE             ((unsigned)0x1 << 24)
  #define ERR_SL_SYN2                ((unsigned)0x1 << 25)
  #define ERR_MB_ADD                 ((unsigned)0x1 << 26)
  #define ERR_MB_TBL                  ((unsigned)0x1 << 27)
  #define ERR_MOT_TP                  ((unsigned)0x1 << 28)
  #define ERR_MOT_CD                 ((unsigned)0x1 << 29)
  #define ERR_MKB                       ((unsigned)0x1 << 30)
  #define ERR_MCIDCT_BSY          ((unsigned)0x1 << 31)

  // Error Type Define
  #define ETP_OUT_PIC                 18
  #define ETP_BL_PIX                    1
  #define ETP_CBP_NS                   2
  #define ETP_DCT_Y_TAB             3
  #define ETP_DCT_C_TAB             4
  #define ETP_SL_SYN                   5
  #define ETP_EOB_SYN                6
  #define ETP_MB_ADD_INC_TAB   7
  #define ETP_CBP_TAB                 8
  #define ETP_INS_SL                    9
  #define ETP_SL_QSCALE             10
  #define ETP_SL_SYN2                 11
  #define ETP_MB_ADD                  12
  #define ETP_MB_TBL                   13
  #define ETP_MOT_TP                   14
  #define ETP_MOT_CD                  15
  #define ETP_MKB                        16
  #define ETP_MCIDCT_BSY           17

#define RW_MBSTART_DCAC_SWITCH     0x1F0

#define RW_VLD_PARA        0x88
#define RW_VLD_PROC        0x8C
    #define VLD_SSCBIT          ((unsigned)0x1 << 16)
    #define VLD_SSCBYTE      ((unsigned)0x1 << 17)
    #define VLD_ABRT            ((unsigned)0x1 << 18)
    #define VLD_PSUP            ((unsigned)0x1 << 19)
    #define VLD_INIFET          ((unsigned)0x1 << 20)
    #define VLD_MBDATA       ((unsigned)0x1 << 21)  // for 1389 MP ECO
    #define VLD_PDHW           ((unsigned)0x1 << 22)
    #define VLD_INIBR           ((unsigned)0x1 << 23)
    #define VLD_PERRCON      ((unsigned)0x1 << 24)
    #define VLD_RTERR          ((unsigned)0x1 << 25)
    #define VLD_EOFR            ((unsigned)0x1 << 26)
    #define VLD_IGBCL          ((unsigned)0x1 << 27)
    #define VLD_DECTOP       ((unsigned)0x1 << 29)
    #define VLD_DECBTM       ((unsigned)0x2 << 29)
    #define VLD_FPRBS          ((unsigned)0x3 << 29)
    #define VLD_B21EN          ((unsigned)0x1 << 31)

#define RW_VLD_PICSZ           0x90
#define RW_VLD_MBROWPRM  0x94
#define RW_VLD_DIGMBSA      0x98
#define RW_VLD_SCALE          0x9C
    #define H_SCALE_1_1    0
    #define H_SCALE_1_2    1
    #define H_SCALE_1_4    2
    #define H_SCALE_3_4    3
    #define H_SCALE_3_8    4
    #define V_SCALE_1_1    0
    #define V_SCALE_1_2    1
    #define V_SCALE_1_4    2

#define RW_VLD_NEWMP2VLDMD                  0x9C

#define RW_VLD_TABLIM         0xA0
#define RW_VLD_TABLNIM       0xA4
#define RW_VLD_DIGMBYOFF   0xA8
#define RW_VLD_PSUPCTR       0xAC
#define RW_VLD_RPTR             0xB0
#define RW_VLD_VSTART         0xB4
#define RW_VLD_VEND            0xB8
#define RO_VLD_BLKFIN          0xBC

#define RW_VLD_RDY_SWTICH   0xC0
   #define READY_TO_RISC         (0x1 << 17)
   #define READY_TO_RISC_1     (0x1 << 18)
   #define READY_TO_RISC_2     (0x1 << 20)
#define RW_VLD_PWRSAVE        0xC4
#define RW_VLD_PIC_W_MB        0xC8
#define RW_VLD_IDLEMAX                      0xCC
#define RW_VLD_ASYNC            0xD0
  #define VLD_WR_ENABLE      ((unsigned)0x1 << 18)
#define RW_VLD_TIMEOUT         0xDC
#define RO_VLD_FSBI               0xE0
#define RO_VLD_ERRFG             0xE4
#define RO_VLD_MP2DECERR                      0x114
#define RO_VLD_FETCHOK         0xE8
  #define VLD_FETCH_OK            ((unsigned)0x1 << 0)
  #define VLD_DRAM_REQ_FIN    ((unsigned)0x1 << 2)

#define RO_VLD_VBAR          0xEC
#define RW_VLD_VBST                         0xEC
    #define BS_PRE1_EN                      (1 << 28)

#define RO_VLD_INPUTWND  0xF0
#define RO_VLD_SRAMCTRL  0xF4
    #define AA_FIT_TARGET_SCLK          (1<<0)
    #define AA_FIT_TARGET_D             (1<<1)
    #define PROCESS_FLAG                (1<<15)

#define RO_VLD_VWPTR       0xF8
#define RO_VLD_VRPTR        0xFC
#define RW_VLD_WAITT       0x100
#define WO_VLD_FDEC        0x104
    #define VLD_PIC_COMPLETE        ((unsigned)0x1 << 0)
    #define VLD_RELOAD_INTRA        ((unsigned)0x1 << 8)
    #define VLD_RELOAD_NONINTRA  ((unsigned)0x1 << 9)
    #define VLD_RST_QMATRIX         ((unsigned)0x1 << 10)

#define WO_VLD_SRST         0x108
#define WO_VLD_WPTR        0x110
  #define  VLD_CLEAR_PROCESS_EN   ((unsigned)0x1 << 0)
  #define VLD_RISC_WR_EN      ((unsigned)0x1 << 1)
#define WPTR_ALIGN          0x10
#define WPTR_ALIGN_MARK     0xf
  
#define WO_VLD_BBUFRST   0x10C
#define RO_VLD_SUM           0x114
#define RW_VLD_DIGEST_BOUNDARY  0x1A4
#define VLD_RESYNC_MARK_ECO    0x2


// HHKuo's
#define RW_VLD_2ND_RDY_SWTICH        0x4C0

#define RO_VLD_AV_2ND_BARL                    0x800
#define RO_VLD_AV_2ND_UE                        0x888
#define RO_VLD_AV_2ND_SE                        0x88c         // 2's complement
#define RW_VLD_AV_2ND_CTRL                    0x884
#define RO_VLD_AV_BARL           0x00
#define RW_VLD_AV_CTRL           0x84


#define  RW_VLD_RESYNC_MARK      0x254

/* JPEG decoding */
#define WO_VLD_RDY                  0x114
// *********************************************************************
// H.264 related VLD Registers define
// *********************************************************************
// KB's scaling list
#define RW_VLD_SCL_ADDR      0x260
#define VLD_NONINTRA       (0x1 << 6)
#define VLD_READ_QMATRIX   (0x1 << 8)
#define VLD_WRITE_QMATRIX  (0x1 << 9)
#define RW_VLD_SCL_DATA      0x264

#define RW_VLD_PIC_MB_SIZE_M1               0x68
#define VLD_TOP_PIC_HEIGHT_IN_MBS_POS   16

// *********************************************************************
// MC Registers define
// *********************************************************************
#define RW_MC_OPBUF       0x24
#define RO_MC_MBX           0x28
#define RO_MC_MBY           0x2C



#define RW_MC_ADDRSWAP   0x90
#define ADDRSWAP_OFF            (0x2 << 1)

#define RW_MC_UMV_PIC_WIDTH     0x208
#define RW_MC_UMV_PIC_HEIGHT    0x20C


// post-processing registers
#define RW_MC_PP_ENABLE        0x220
#define RW_MC_PP_Y_ADDR        0x224
#define RW_MC_PP_C_ADDR        0x228
#define RW_MC_PP_MB_WIDTH    0x22C

#define RW_MC_PP_DBLK_MODE  0x238
   #define DBLK_Y                        (0x1 << 1)
   #define DBLK_C                        (0x1)
#define RW_MC_PP_DRING_THD      0x248
#define RW_MC_PP_INTERLACE       0x24C
   #define INTERLACE_DRING           (0x1 << 8)
   #define INTERLACE_DBLK             (0x1)
#define RW_MC_PP_WB_BY_POST    0x250
#define RW_MC_PP_LOW_BW           0x254
#define RW_MC_PP_SEQ                  0x258
   #define DBLK_THEN_DRING           (0x0)
   #define DRING_THEN_DBLK           (0x1)
#define RW_MC_PP_X_RANGE          0x260
#define RW_MC_PP_Y_RANGE          0x264
#define RW_MC_PP_MODE               0x268
   #define WMV_MODE                     (0x0)
   #define H264_MODE                     (0x1)

#define RW_MC_WMV9_PRE_PROC    0x270
    #define PP_NO_SCALE                  (0x0)
    #define PP_SCALE_DOWN             (0x1)
    #define PP_SCALE_UP                  (0x1 << 1)
#define RW_MC_WMV9_ICOMP_EN    0x274
#define ICOMP_C_OFF                 (0x0)
#define ICOMP_C_EN                   (0x1)
#define ICOMP_Y_OFF                 (0x0 << 1)
#define ICOMP_Y_EN                   (0x1 << 1)
#define RW_MC_ISCALE1_X1            0x278
#define RW_MC_ISCALE1_X3            0x27C
#define RW_MC_ISCALE1_X5            0x280
#define RW_MC_ISCALE1_X7            0x284
#define RW_MC_ISCALE1_X9            0x288
#define RW_MC_ISCALE1_X11           0x28C
#define RW_MC_ISCALE1_X13           0x290
#define RW_MC_ISCALE1_X15           0x294
#define RW_MC_YSHIFT_OFF1           0x298
#define RW_MC_CSHIFT_OFF1           0x29C
#define RW_MC_FILTER_TYPE           0x2A0
    #define C_BILINEAR                     (0x0)
    #define C_BICUBIC                      (0x1)
    #define Y_BILINEAR                     (0x0 << 1)
    #define Y_BICUBIC                      (0x1 << 1)
#define RW_MC_WRITE_BUS_TYPE    0x2A4
    #define CLIP_0_255_TYPE            0
    #define UNCLIP_TYPE                   1
#define RW_MC_INTRA_BLK_ADD128 0x2a8
    #define ADD128_OFF                    0
    #define ADD128_ON                      1
#define RW_MC_FAST_UVMC             0x2B4
    #define FAST_UVMC_EN                (0x1)
#define RW_MC_OVL_SMTH_FILTER   0x2BC
    #define OVL_OFF                         (0x0)
    #define OVL_EN                           (0x1)
#define RW_MC_COND_OVL_FILTER   0x2C0
    #define COND_OVL_OFF               (0x0)
    #define COND_OVL_EN                 (0x1)
#define RW_MC_PP_DBLK_OPT          0x2C4
    #define CHK_INTRA0                     (0x1)
    #define CHK_BLK3                        (0x1 << 8)
    #define DBLK_FLG_4x4                 (0x1 << 16)
    #define VC1_ALL_SET_ONE            (CHK_INTRA0 + CHK_BLK3 + DBLK_FLG_4x4)
    #define NOT_VC1_ALL_SET_ZERO   (0)
#define RW_MC_PP_DBLK_Y_ADDR     0x2C8
#define RW_MC_PP_DBLK_C_ADDR     0x2CC
#define RW_MC_WMV8_MIX_PEL        0x2D0
    #define MIX_PEL_MC_EN               (0x0)
#define RW_MC_RNG_PARA                0x2E4 //This register is not used in current hardware any more
#define RW_MC_ISCALE2_X1             0x2EC
#define RW_MC_ISCALE2_X3             0x2F0
#define RW_MC_ISCALE2_X5             0x2F4
#define RW_MC_ISCALE2_X7             0x2F8
#define RW_MC_ISCALE2_X9             0x2FC
#define RW_MC_ISCALE2_X11           0x300
#define RW_MC_ISCALE2_X13           0x304
#define RW_MC_ISCALE2_X15           0x308
#define RW_MC_YSHIFT_OFF2           0x30C
#define RW_MC_CSHIFT_OFF2           0x310
#define RW_MC_ICOMP_TYPE            0x314
    #define NO_ICOMP                 0
    #define FRAME_ICOMP           1
    #define TOP_FLD_ICOMP        2
    #define BTM_FLD_ICOMP        3
    #define BOTH_FLD_ICOMP      4
#define RW_MC_ISCALE3_X1       0x318
#define RW_MC_ISCALE3_X3       0x31C
#define RW_MC_ISCALE3_X5       0x320
#define RW_MC_ISCALE3_X7       0x324
#define RW_MC_ISCALE3_X9       0x328
#define RW_MC_ISCALE3_X11      0x32C
#define RW_MC_ISCALE3_X13      0x330
#define RW_MC_ISCALE3_X15      0x334
#define RW_MC_YSHIFT_OFF3      0x338
#define RW_MC_CSHIFT_OFF3      0x33C
#define RW_MC_ICOMP2_EN         0x340
#define RW_MC_ISCALE4_X1        0x344
#define RW_MC_ISCALE4_X3        0x348
#define RW_MC_ISCALE4_X5        0x34C
#define RW_MC_ISCALE4_X7        0x350
#define RW_MC_ISCALE4_X9        0x354
#define RW_MC_ISCALE4_X11      0x358
#define RW_MC_ISCALE4_X13      0x35C
#define RW_MC_ISCALE4_X15      0x360
#define RW_MC_YSHIFT_OFF4      0x364
#define RW_MC_CSHIFT_OFF4      0x368
#define RW_MC_ISCALE5_X1        0x36C
#define RW_MC_ISCALE5_X3        0x370
#define RW_MC_ISCALE5_X5        0x374
#define RW_MC_ISCALE5_X7        0x378
#define RW_MC_ISCALE5_X9        0x37C
#define RW_MC_ISCALE5_X11      0x380
#define RW_MC_ISCALE5_X13      0x384
#define RW_MC_ISCALE5_X15      0x388
#define RW_MC_YSHIFT_OFF5      0x38C
#define RW_MC_CSHIFT_OFF5      0x390
#define RW_MC_BWD_ICOMP_FLD    0x394
    #define BWD_TOP_FLD           (0x0)
    #define BWD_BTM_FLD          (0x1)
#define RW_MC_SAME_ICOMP       0x398
    #define NO_USE_SAME_ICOPM1_FOR_FRAME    (0x0)
    #define USE_SAME_ICOMP1_FOR_FRAME          (0x1)
#define RW_MC_REF_PIC_TYPE     0x39C
  #define BWD_PROG_REF_PIC     (0x0)
  #define BWD_INTLCE_REF_PIC   (0x1)
  #define FWD_PROG_REF_PIC     (0x0 << 16)
  #define FWD_INTLCE_REF_PIC   (0x1 << 16)
#define RW_MC_ISCALE6_X1       0x3A0
#define RW_MC_ISCALE6_X3       0x3A4
#define RW_MC_ISCALE6_X5       0x3A8
#define RW_MC_ISCALE6_X7       0x3AC
#define RW_MC_ISCALE6_X9       0x3B0
#define RW_MC_ISCALE6_X11     0x3B4
#define RW_MC_ISCALE6_X13     0x3B8
#define RW_MC_ISCALE6_X15     0x3BC
#define RW_MC_YSHIFT_OFF6     0x3C0
#define RW_MC_CSHIFT_OFF6     0x3C4
#define RW_MC_SAME_REF_PIC   0x3CC
#define RW_MC_P_LIST0             0x3DC // ~0x458
#define RW_MC_B_LIST0             0x45C // ~0x4d8
#define RW_MC_B_LIST1             0x4DC // ~0x558
#define RW_MC_PIC1Y_ADD                     0x3E0  // 248x4
#define RW_MC_PIC2Y_ADD                     0x3E4
#define RW_MC_PIC3Y_ADD                     0x3E8
#define RW_AMC_CBCR_OFFSET  0x55C
#define RW_AMC_Y_OUT_ADDR   0x560
#define RW_AMC_P_LIST0_FLD    0x564
#define RW_AMC_B_LIST0_FLD    0x568
#define RW_AMC_B_LIST1_FLD    0x56C
#define RW_MC_WRAPPER_SWITCH   0x57C
#define RW_MC_IRATIO                 0x690    //RM Codec
#define RO_MC_DRAM_PEAK         0x770
#define RW_MC_VLD_WRAPPER_ADDR              0x93C//4*591
#define MC_VLD_WRAPPER_READ         (0x0ul<<16)
#define MC_VLD_WRAPPER_WRITE        (0x1ul<<16)

#define RW_MC_VLD_WRAPPER_DATA              0x940//4*592

#define RW_MC_PIC_W_MB                      0x980
#define RW_MC_MVDCAC_SEL                    0x80C
#define RW_MC_DIR_VLDWRAP                   0x93C
#define RW_MC_DIR_VLDWRAP2                  0x940
#define RW_MC_DIR_VLDWRAP3                  0x944


// *********************************************************************
// NBM MC Registers define
// *********************************************************************
#define RW_MC_NBM_CTRL         0x768
    #define RW_MC_NBM_OFF      (0x1 << 8)

// *********************************************************************
// DDR3 Switch MC Registers define
// *********************************************************************
#define RW_MC_DDR_CTRL0         0x7FC
#define RW_MC_DDR_CTRL1         0x800
#define RW_MC_DDR3_EN             0x834

// *********************************************************************
// PP Dram Protect Mode
// *********************************************************************
#define RW_MC_PP_DRAM_PROTECT_UPPER_BOUND    0x850
#define RW_MC_PP_DRAM_PROTECT_LOWER_BOUND    0x854
#define RW_MC_PP_DRAM_PROTECT_EN             0x858

// *********************************************************************
// AVC VLD Registers define
// *********************************************************************
// Barral Shifter from 0x00 - 0x80
#define RO_AVLD_BARL      0x00
#define RW_AVLD_CTRL      0x84
   #define AVC_EN              (0x1)
   #define AVC_DEC_CYCLE_EN          (0x1<<13)
   #define AVC_ERR_BYPASS          ((unsigned)(0x3) << 14)
   #define AVC_NON_SPEC_SWITCH          ((unsigned)(0x1) << 15)
   #define AVC_ERR_CONCEALMENT          ((unsigned)(0x2) << 22)
   #define AVC_ERR_ONLY1MC_START    (0x1 << 25)
   #define AVC_ERR_SKIP_PIC_END     (0x1 << 26)
   #define AVLD_MEM_PAUSE_MOD_EN (0x1<<27)
   #define AVC_RDY_WITH_CNT                 ((unsigned)(0x3) << 1)  //in search start code, it should be set as 3
   #define AVC_RDY_CNT_THD      (0x0 << 3)
   #define AVC_RBSP_CHK_INV             ((unsigned)(0x0) << 28)
   #define AVC_SUM6_APPEND_INV       ((unsigned)(0x1) << 29)
   #define AVC_READ_FLAG_CHK_INV    ((unsigned)(0x0) << 30)
   #define AVC_NOT_CHK_DATA_VALID  ((unsigned)(0x1) << 31)
#define RW_AVLD_SPS              0x88
#define RW_AVLD_PIC_SIZE      0x8C
#define RW_AVLD_PPS_1           0x90
#define RW_AVLD_PPS_2           0x94
#define RW_AVLD_SHDR_1         0x98
#define RW_AVLD_SHDR_2         0x9C
#define RW_AVLD_MAX_PIC_NUM     0xA0
#define RW_AVLD_CUR_PIC_NUM      0xA4
#define RW_AVLD_REORD_P0_RPL     0xA8
#define RW_AVLD_REORD_B0_RPL     0x128
#define RW_AVLD_REORD_B1_RPL     0x1A8
#define RW_AVLD_RESET_PIC_NUM    0x228
   #define RESET_PIC_NUM 0x1
#define RW_AVLD_RPL_REORD                 0x22C
#define RW_AVLD_WEIGHT_PRED_TBL      0x230
#define RW_AVLD_INIT_CTX_SRAM           0x234
#define RW_AVLD_PROC                          0x238
#define RO_AVLD_UE                               0x23C
#define RO_AVLD_SE                               0x240         // 2's complement
#define RW_AVLD_PRED_ADDR                 0x244
#define RO_AVLD_STATUS                        0x25c
   #define RO_AVLD_STALL                       0x2000000
#define RO_AVLD_TIMEOUT_THD              0x268
   #define AVLD_TIMEOUT_THD                 0xffffffff
#define RO_AVLD_ERR_MESSAGE              0x270
#define RO_AVLD_COMPLETE                   0x274
#define RO_AVLD_STATE_INFO                 0x278
     #define AVLD_PIC_FINISH                ((unsigned)(0x1) << 31)
#define RW_AVLD_2ND_BARL_CTRL          0x27C
     #define AVLD_2ND_BARL_EN 0x3
#define RW_AVLD_ERR_MASK                   0x280
     #define AVLD_MB_END_CHK                ((unsigned)(0x1) << 14)
     #define AVLD_4BLOCKS_SKIP_CHK      ((unsigned)(0x1) << 21)
#define RO_AVLD_ERR_ACCUMULATOR      0x284
    #define NO_NEXT_START_CODE          ((unsigned)(0x1) << 3)
    #define CABAC_ALIGN_BIT_ERR          ((unsigned)(0x1) << 8)
    #define CABAC_ZERO_WORD_ERR       ((unsigned)(0x1) << 14)
    #define ERROR_MASK                  0xBFDFB2F7
#define RO_AVLD_DEC_CYCLE                 0x288
#define RW_AVLD_RESET_SUM                 0x28C
     #define AVLD_RESET_SUM_OFF   0x0
     #define AVLD_RESET_SUM_ON     0x1
#define RW_AVLD_MC_BUSY_THRESHOLD  0x290

#define RW_AVLD_RM03R                         0x2C8
     #define RO_ALVD_FIND_03                  ((unsigned)(0x1) << 11)
     #define VIEW_REORDER_SWITCH           ((unsigned)(0x1) << 13)
     #define MVC_SWITCH                           ((unsigned)(0x1) << 14)
     #define HEADER_EXT_SWITCH                           ((unsigned)(0x1) << 16)
     #define REORDER_MVC_SWITCH           ((unsigned)(0x1) << 17)
#define RW_AVLD_ERC_CTRL    0x2D0
    #define ERC_DISABLE     0
    #define ERC_ENABLE      1
    #define DERC            (0x1 << 1)
    #define ERC_MODE0       (0x0 << 8)
    #define ERC_MODE1       (0x1 << 8)
    #define ERC_MODE2       (0x2 << 8)
#define RW_AVLD_ERC_DED_ERR_TYPE 0x2D4
    #define ERC_DED_ERR_TYPE 0x0000C210
#define RW_AVLD_FSSR                            0x2D8
     #define FW_SEARCH_START_CODE       ((unsigned)(0x1) << 0)


#define RO_AVLD_2ND_BARL                    0x800
#define RO_AVLD_2ND_UE                        0x888
#define RO_AVLD_2ND_SE                        0x88c         // 2's complement
#define RW_AVLD_2ND_CTRL                    0x884

// *********************************************************************
// AVC MV Registers define
// *********************************************************************
#define RW_AMV_P_REF_PARA        0             // ~0x7C
#define RW_AMV_B0_REF_PARA      0x80      // ~0xFC
#define RW_AMV_B1_REF_PARA      0x100    // ~0x17C
#define RW_AMV_B1_REF_ADDR      0x180    // ~0x1FC
#define RW_AMV_MV_BUF_ADDR      0x180
#define RW_AMV_CURR_POC           0x200
#define RW_AMV_CURR_TFLD_POC  0x204
#define RW_AMV_CURR_BFLD_POC  0x208
#define RW_AMV_EC_SETTING           0x210
    #define AVOID_HANG              (0x1 << 2)
    #define SKIP_MODE0              0
    #define SKIP_MODE1              1
    #define SKIP_MODE2              2
    #define SKIP_MODE3              3
#define RW_AMV_WR_ADDR            0x20C
#define RW_AMV_REDUCE_BMV      0x220
      #define EN_AMV_REDUCE_BMV      (0x1 << 5)
      #define EN_AMV_ALLEG_MVC_CFG (0x1 << 6)
// *********************************************************************
// AVC FilmGrain Registers define
// *********************************************************************
#define RW_FGT_MODE                    0x00
    #define FGT_EN  0x1 << 0
    #define FGT_SCR_PP  0x1 << 1
    #define FGT_VDSCL_BUSY_EN 0x1<<4
#define RW_FGT_SEED_ADDR           0x04
#define RW_FGT_SEI_ADDR_A          0x08
#define RW_FGT_SEI_ADDR_B          0x0C
#define RW_FGT_DATABASE_ADDR   0x10
#define RW_FGT_OUT_Y_ADDR         0x14
#define RW_FGT_OUT_C_ADDR         0x18
#define RW_FGT_IN_Y_ADDR           0x1C
#define RW_FGT_IN_C_ADDR           0x20
#define RW_FGT_MB_SIZE               0x24
#define RW_FGT_DRAM_CTRL           0x28
#define RW_FGT_SEI_CTRL_A          0x2c
    #define FGT_CANCLE_FLG_A  0x1 << 0
#define RW_FGT_SEI_CTRL_B          0x30
#define RW_FGT_CTRL_STATE          0x34

// *********************************************************************
// Video Constant define
// *********************************************************************
// Referenc Buf / B Buf / Digest Buf / Post Processing Buf Index
#define MPV_REF_BUF_0       0
#define MPV_REF_BUF_1       1
#define MPV_B_BUF_0         2
#define MPV_B_BUF_1         3

#define MC_DIG_BUF          2

// Picture Field control for MC
#define MC_TOP_FLD          0
#define MC_BOTTOM_FLD       1

#define MC_2ND_FLD_OFF      0
#define MC_2ND_FLD_ON       1


// *********************************************************************
// Video Decoder Macros
// *********************************************************************
#define vWriteReg(dAddr, dVal)  *(volatile UINT32 *)(0xF0000000+ dAddr) = dVal
#define u4ReadReg(dAddr)         *(volatile UINT32 *)(0xF0000000+ dAddr)

#define SPM_BASE_VA (0xF0000000 + 0x48000)
#define SPM_READ32(REG)    *(volatile UINT32 *)(SPM_BASE_VA+ REG)
#define SPM_WRITE32(VAL, REG)  *(volatile UINT32 *)(SPM_BASE_VA+ REG) = VAL


// *********************************************************************
//  Common Video Decoder HW Functions
// *********************************************************************
extern void vVDecWriteVLD(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val);
extern UINT32 u4VDecReadVLD(UINT32 u4VDecID, UINT32 u4Addr);
extern void vVDecWriteVLDTOP(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val);
extern void vVDecWriteAVCMV(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val);
extern UINT32 u4VDecReadAVCMV(UINT32 u4VDecID, UINT32 u4Addr);
extern UINT32 u4VDecReadVLDTOP(UINT32 u4VDecID, UINT32 u4Addr);
extern void vVDecWriteMC(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val);
extern UINT32 u4VDecReadMC(UINT32 u4VDecID, UINT32 u4Addr);
extern void vVDecWriteDV(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val);
extern void vVDecWriteVLDTOP(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val);
extern UINT32 u4VDecReadVLDTOP(UINT32 u4VDecID, UINT32 u4Addr);

extern void vVDecWriteCRC(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val);
extern UINT32 u4VDecReadCRC(UINT32 u4VDecID, UINT32 u4Addr);

#endif

