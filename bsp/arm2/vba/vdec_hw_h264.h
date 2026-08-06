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
#ifndef _VDEC_HW_H264_H_
#define _VDEC_HW_H264_H_
//#include "typedef.h"
//#include "vdec_info_common.h"
#include "vdec_hal_if_h264.h"
#include "x_printf.h"    

//***********************************************************************************
//*
//*     AVC_VLD Registers Base Define (AVC_VLD_REG_OFFSET0/AVC_VLD_REG_OFFSET1)
//*
//***********************************************************************************
// Barral Shifter from 0x00 - 0x80

#define HAL_HANDLE_OK 0x1

#define INVLAID_HANDLE_VALUE 0x80000001
#define INIT_BARRELSHIFTER_FAIL 0x80000002

#define RO_AVLD_BARL                        0x00
#define RW_AVLD_CTRL                        0x84
    #define AVC_EN                      (0x1)
    #define AVC_DEC_CYCLE_EN            (0x1<<13)
    #define AVC_ERR_BYPASS              ((unsigned)(0x3) << 14)
    #define AVC_NON_SPEC_SWITCH         ((unsigned)(0x1) << 15)
    #define AVC_ERR_CONCEALMENT         ((unsigned)(0x2) << 22)
    #define AVLD_MEM_PAUSE_MOD_EN       (0x1<<27)

    #define AVC_RDY_WITH_CNT            ((unsigned)(0x3) << 1)  //in search start code, it should be set as 3
    #define AVC_RDY_CNT_THD             (0x0 << 3)
    #define AVC_RBSP_CHK_INV            ((unsigned)(0x0) << 28)
    #define AVC_SUM6_APPEND_INV         ((unsigned)(0x1) << 29)
    #define AVC_READ_FLAG_CHK_INV       ((unsigned)(0x0) << 30)
    #define AVC_NOT_CHK_DATA_VALID      ((unsigned)(0x1) << 31)

#define RW_AVLD_SPS                         0x88
#define RW_AVLD_PIC_SIZE                    0x8C
    #define AVLD_PIC_HEIGHT_IN_MBS_POS 8
    #define FW_FST_MB_IN_SLICE_MASK  0x1FFFF

#define RW_AVLD_PPS_1                       0x90
#define RW_AVLD_PPS_2                       0x94
#define RW_AVLD_SHDR_1                      0x98
#define RW_AVLD_SHDR_2                      0x9C
#define RW_AVLD_MAX_PIC_NUM                 0xA0
#define RW_AVLD_CUR_PIC_NUM                 0xA4
#define RW_AVLD_REORD_P0_RPL                0xA8
#define RW_AVLD_REORD_B0_RPL                0x128
#define RW_AVLD_REORD_B1_RPL                0x1A8
#define RW_AVLD_RESET_PIC_NUM               0x228
    #define RESET_PIC_NUM               0x1

#define RW_AVLD_RPL_REORD                   0x22C
#define RW_AVLD_WEIGHT_PRED_TBL             0x230
#define RW_AVLD_INIT_CTX_SRAM               0x234
#define RW_AVLD_PROC                        0x238
#define RO_AVLD_UE                          0x23C
#define RO_AVLD_SE                          0x240         // 2's complement
#define RW_AVLD_PRED_ADDR                   0x244
#define RO_AVLD_STATUS                      0x25c
    #define RO_AVLD_STALL                0x2000000

#define RO_AVLD_TIMEOUT_THD                 0x268
    #define AVLD_TIMEOUT_THD             0xffffffff

#define RO_AVLD_ERR_MESSAGE                 0x270
#define RO_AVLD_COMPLETE                    0x274
#define RO_AVLD_STATE_INFO                  0x278
    #define AVLD_PIC_FINISH             ((unsigned)(0x1) << 31)

#define RW_AVLD_2ND_BARL_CTRL               0x27C
    #define AVLD_2ND_BARL_EN            0x3

#define RW_AVLD_ERR_MASK                    0x280
    #define AVLD_MB_END_CHK             ((unsigned)(0x1) << 14)
    #define AVLD_4BLOCKS_SKIP_CHK       ((unsigned)(0x1) << 21)


#define RO_AVLD_ERR_ACCUMULATOR             0x284
    #define NO_NEXT_START_CODE          ((unsigned)(0x1) << 3)
    #define CABAC_ALIGN_BIT_ERR         ((unsigned)(0x1) << 8)
    #define CABAC_ZERO_WORD_ERR         ((unsigned)(0x1) << 14)

#define RO_AVLD_DEC_CYCLE                   0x288
#define RW_AVLD_RESET_SUM                   0x28C
    #define AVLD_RESET_SUM_OFF          0x0
    #define AVLD_RESET_SUM_ON           0x1

#define RW_AVLD_MC_BUSY_THRESHOLD           0x290
#define RW_AVLD_RM03R                       0x2C8
    #define RO_ALVD_FIND_03             ((unsigned)(0x1) << 11)
    #define VIEW_REORDER_SWITCH         ((unsigned)(0x1) << 13)
    #define MVC_SWITCH                  ((unsigned)(0x1) << 14)
    #define HEADER_EXT_SWITCH           ((unsigned)(0x1) << 16)
    #define REORDER_MVC_SWITCH          ((unsigned)(0x1) << 17)

#define RW_AVLD_FSSR                        0x2D8
     #define FW_SEARCH_START_CODE       ((unsigned)(0x1) << 0)

#define RO_AVLD_2ND_BARL                    0x800
#define RO_AVLD_2ND_UE                      0x888
#define RO_AVLD_2ND_SE                      0x88c         // 2's complement
#define RW_AVLD_2ND_CTRL                    0x884




//***********************************************************************************
//*
//*     AVC_MV Registers Base Define (AVC_MV_REG_OFFSET0/AVC_MV_REG_OFFSET1)
//*
//***********************************************************************************
#define RW_AMV_P_REF_PARA                   0             // ~0x7C
#define RW_AMV_B0_REF_PARA                  0x80      // ~0xFC
#define RW_AMV_B1_REF_PARA                  0x100    // ~0x17C
#define RW_AMV_B1_REF_ADDR                  0x180    // ~0x1FC
#define RW_AMV_CURR_POC                     0x200
#define RW_AMV_CURR_TFLD_POC                0x204
#define RW_AMV_CURR_BFLD_POC                0x208
#define RW_AMV_WR_ADDR                      0x20C
#define RW_AMV_REDUCE_BMV                   0x220
    #define EN_AMV_REDUCE_BMV           (0x1 << 5)
    #define EN_AMV_ALLEG_MVC_CFG        (0x1 << 6)


// *********************************************************************
//  Video Decoder HW Functions
// *********************************************************************
extern void vVDecWriteAVCVLD(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val);
extern UINT32 u4VDecReadAVCVLD(UINT32 u4VDecID, UINT32 u4Addr);
//extern void vVDecWriteAVCMV(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val);
//extern UINT32 u4VDecReadAVCMV(UINT32 u4VDecID, UINT32 u4Addr);
extern UINT32 u4VDecAVCVLDShiftBits(UINT32 u4BSID, UINT32 u4VDecID);
extern UINT32 u4VDecAVCVLDGetBitS(UINT32 u4BSID,UINT32 u4VDecID,UINT32 dShiftBit);
extern BOOL fgInitH264BarrelShift2(UINT32 u4VDecID, VDEC_HAL_H264_BS_INIT_PRM_T *prH264BSInitPrm);
extern BOOL fgInitH264BarrelShift1(UINT32 u4VDecID, VDEC_HAL_H264_BS_INIT_PRM_T *prH264BSInitPrm);
extern void vInitFgtHWSetting(UINT32 u4VDecID, VDEC_INFO_H264_INIT_PRM_T *prH264VDecInitPrm);
extern UINT32 u4VDecReadH264VldRPtr(UINT32 u4BSID, UINT32 u4VDecID, UINT32 *pu4Bits, UINT32 u4VFIFOSa);

#endif

