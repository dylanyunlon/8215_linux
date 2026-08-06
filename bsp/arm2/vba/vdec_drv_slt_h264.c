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
#ifndef _VDEC_SLT_H264_C_
#define _VDEC_SLT_H264_C_

#include "vdec_hw_common.h"
#include "vdec_hw_h264.h"
#include "vdec_drv_h264_info.h"
#include "preloader_common.h"
#include "x_printf.h"
#include "reserved_memory.h"
#include "vdec_drv_slt_h264.h"

#define AVC_HAL_SUPPORT_SLT

#ifdef AVC_HAL_SUPPORT_SLT
#define  HW_DECODE_TIME_OUT  0x800000

#define RESRC_SIZE 0x8000
static UINT32 AVC_SLT_V_FIFO_SZ = 0x100000;//0x1400000
static UINT32 AVC_SLT_DPB_SZ;//     0xC00000//0x01200000
#define AVC_SLT_PRED_SZ    23*1024
#define fgIsNonRefFBuf(u4InstID, arg) ((_ptFBufInfo[u4InstID][arg].ucFBufRefType == NREF_PIC) && (_ptFBufInfo[u4InstID][arg].ucTFldRefType == NREF_PIC) && (_ptFBufInfo[u4InstID][arg].ucBFldRefType == NREF_PIC))
#define fgSLTIsIDRPic(u4InstID)  ((_ucNalUnitType[u4InstID] == IDR_SLICE))
#define fgSLTIsFrmPic(u4InstID)  ((_tVDecPrm[u4InstID].ucPicStruct == FRAME))

UINT32 AVC_SLT_DPB_BUF_ADDR;
UINT32 AVC_SLT_PRED_BUF_ADDR;

static UCHAR _u4NalRefIdc[2];  
static UCHAR _ucNalUnitType[2];
static UINT32 _u4AVCCurrPicStartAddr[2];
static UCHAR *_pucAVCDecWorkBuf[2];
static BYTE *_pbAVCDPBBuf = NULL;
static UINT32 _pu4AVCDPBBUF;


static VDEC_INFO_H264_SLICE_HDR_T _rH264SliceHdr[2];
static VDEC_INFO_H264_SPS_T _rH264SPS[2][32];
static VDEC_INFO_H264_PPS_T _rH264PPS[2][256];
static VDEC_INFO_H264_SEI_T _rESI[2];
static VDEC_INFO_H264_FBUF_INFO_T *_ptCurrFBufInfo[2];
static VDEC_INFO_H264_FBUF_INFO_T _ptFBufInfo[2][17] = {0};
static VDEC_INFO_H264_REF_PIC_LIST_T _ptRefPicList[2][6];
static VDEC_INFO_H264_P_REF_PRM_T _arPRefPicListInfo[2];
static VDEC_INFO_H264_B_REF_PRM_T _arBRefPicListInfo[2];

VDEC_INFO_DEC_PRM_T _tVDecPrm[2];

CHAR quant_intra_default[16] = {
    6, 13, 20, 28,
   13, 20, 28, 32,
   20, 28, 32, 37,
   28, 32, 37, 42
};

CHAR quant_inter_default[16] = {
    10, 14, 20, 24,
    14, 20, 24, 27,
    20, 24, 27, 30,
    24, 27, 30, 34
};

CHAR quant8_intra_default[64] = {
    6,10,13,16,18,23,25,27,
    10,11,16,18,23,25,27,29,
    13,16,18,23,25,27,29,31,
    16,18,23,25,27,29,31,33,
    18,23,25,27,29,31,33,36,
    23,25,27,29,31,33,36,38,
    25,27,29,31,33,36,38,40,
    27,29,31,33,36,38,40,42
};

CHAR quant8_inter_default[64] = {
    9,13,15,17,19,21,22,24,
    13,13,17,19,21,22,24,25,
    15,17,19,21,22,24,25,27,
    17,19,21,22,24,25,27,28,
    19,21,22,24,25,27,28,30,
    21,22,24,25,27,28,30,32,
    22,24,25,27,28,30,32,33,
    24,25,27,28,30,32,33,35
};

UINT8 pattern[] = {
0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x1E, 0x9A, 0x72, 0x8A, 0x69, 0x80, 0x08, 0x00, 0x00, 0x03,
0x00, 0x08, 0x00, 0x00, 0x03, 0x01, 0x97, 0x00, 0x00, 0x14, 0x06, 0x00, 0x00, 0x2D, 0xC6, 0xC3,
0xF2, 0x94, 0x18, 0x00, 0x00, 0x9B, 0x40, 0x00, 0x01, 0x31, 0x2D, 0x1F, 0x94, 0xA0, 0x50, 0x00,
0x00, 0x00, 0x01, 0x68, 0xC9, 0x08, 0x01, 0xAC, 0x44, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x52, 
0x42, 0x00, 0x67, 0x15, 0x20, 0x00, 0x00, 0x00, 0x01, 0x68, 0x72, 0x42, 0x00, 0x63, 0x19, 0x20, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x24, 0x90, 0x80, 0x17, 0xC5, 0x48, 0x00, 0x00, 0x00, 0x01, 0x68, 
0x2C, 0x90, 0x80, 0x16, 0xC2, 0x12, 0x00, 0x00, 0x00, 0x01, 0x68, 0x34, 0x90, 0x80, 0x15, 0xC2, 
0x52, 0x00, 0x00, 0x00, 0x01, 0x68, 0x3C, 0x90, 0x80, 0x14, 0xCD, 0x20, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x11, 0x24, 0x20, 0x04, 0xF0, 0x84, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x13, 0x24, 0x20, 
0x04, 0xB0, 0x84, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x15, 0x24, 0x20, 0x04, 0x70, 0x94, 0x80, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x17, 0x24, 0x20, 0x04, 0x31, 0x12, 0x00, 0x00, 0x00, 0x01, 0x68, 
0x19, 0x24, 0x20, 0x0F, 0xC2, 0x12, 0x00, 0x00, 0x00, 0x01, 0x68, 0x1B, 0x24, 0x20, 0x0E, 0xCD, 
0x20, 0x00, 0x00, 0x00, 0x01, 0x68, 0x1D, 0x24, 0x20, 0x0D, 0xC7, 0x48, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x1F, 0x24, 0x20, 0x0C, 0xC6, 0x48, 0x00, 0x00, 0x00, 0x01, 0x68, 0x08, 0x49, 0x08, 0x02, 
0xF0, 0x84, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x08, 0xC9, 0x08, 0x02, 0xB1, 0x12, 0x00, 0x00, 
0x00, 0x01, 0x68, 0x09, 0x49, 0x08, 0x02, 0x70, 0x84, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x09, 
0xC9, 0x08, 0x02, 0x31, 0x52, 0x00, 0x00, 0x00, 0x01, 0x68, 0x0A, 0x49, 0x08, 0x07, 0xC7, 0x48, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x0A, 0xC9, 0x08, 0x06, 0xC6, 0x48, 0x00, 0x00, 0x00, 0x01, 0x68, 
0x0B, 0x49, 0x08, 0x05, 0xC2, 0x12, 0x00, 0x00, 0x00, 0x01, 0x68, 0x0B, 0xC9, 0x08, 0x04, 0xC2, 
0x52, 0x00, 0x00, 0x00, 0x01, 0x68, 0x0C, 0x49, 0x08, 0x0F, 0x15, 0x20, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x0C, 0xC9, 0x08, 0x0B, 0x15, 0x20, 0x00, 0x00, 0x00, 0x01, 0x68, 0x0D, 0x49, 0x08, 0x1C, 
0x54, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x0D, 0xC9, 0x08, 0x31, 0xD2, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x0E, 0x49, 0x08, 0x14, 0x25, 0x20, 0x00, 0x00, 0x00, 0x01, 0x68, 0x0E, 0xC9, 0x08, 0x09, 
0x1D, 0x20, 0x00, 0x00, 0x00, 0x01, 0x68, 0x0F, 0x49, 0x08, 0x0D, 0x08, 0x48, 0x00, 0x00, 0x00, 
0x01, 0x68, 0x0F, 0xC9, 0x08, 0x04, 0x4D, 0x20, 0x00, 0x00, 0x00, 0x01, 0x68, 0x04, 0x12, 0x42, 
0x01, 0x50, 0x94, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x04, 0x32, 0x42, 0x01, 0x90, 0x94, 0x80, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x04, 0x52, 0x42, 0x01, 0xD0, 0x84, 0x80, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x04, 0x72, 0x42, 0x00, 0x84, 0x74, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x04, 0x92, 0x42, 
0x00, 0x94, 0x44, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x04, 0xB2, 0x42, 0x00, 0xA4, 0x54, 0x80, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x04, 0xD2, 0x42, 0x00, 0xB4, 0x21, 0x20, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x04, 0xF2, 0x42, 0x00, 0xC4, 0x64, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x05, 0x12, 0x42, 
0x00, 0xD4, 0x44, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x05, 0x32, 0x42, 0x00, 0xE4, 0x44, 0x80, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x05, 0x52, 0x42, 0x00, 0xF4, 0x21, 0x20, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x05, 0x72, 0x42, 0x00, 0x41, 0x15, 0x20, 0x00, 0x00, 0x00, 0x01, 0x68, 0x05, 0x92, 0x42, 
0x00, 0x45, 0x09, 0x48, 0x00, 0x00, 0x00, 0x01, 0x68, 0x05, 0xB2, 0x42, 0x00, 0x49, 0x1D, 0x20, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x05, 0xD2, 0x42, 0x00, 0x4D, 0x08, 0x48, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x05, 0xF2, 0x42, 0x00, 0x51, 0x19, 0x20, 0x00, 0x00, 0x00, 0x01, 0x68, 0x06, 0x12, 0x42, 
0x00, 0x55, 0x15, 0x20, 0x00, 0x00, 0x00, 0x01, 0x68, 0x06, 0x32, 0x42, 0x00, 0x59, 0x08, 0x48, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x06, 0x52, 0x42, 0x00, 0x5D, 0x1D, 0x20, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x06, 0x72, 0x42, 0x00, 0x61, 0x15, 0x20, 0x00, 0x00, 0x00, 0x01, 0x68, 0x06, 0x92, 0x42, 
0x00, 0x65, 0x15, 0x20, 0x00, 0x00, 0x00, 0x01, 0x68, 0x06, 0xB2, 0x42, 0x00, 0x6B, 0x11, 0xA0, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x06, 0xD2, 0x42, 0x00, 0x67, 0x15, 0xA0, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x06, 0xF2, 0x42, 0x00, 0x63, 0x19, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x07, 0x12, 0x42, 
0x00, 0x5F, 0x15, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x07, 0x32, 0x42, 0x00, 0x5B, 0x08, 0x68, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x07, 0x52, 0x42, 0x00, 0x57, 0x09, 0x68, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x07, 0x72, 0x42, 0x00, 0x53, 0x36, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x07, 0x92, 0x42, 
0x00, 0x4F, 0x08, 0x68, 0x00, 0x00, 0x00, 0x01, 0x68, 0x07, 0xB2, 0x42, 0x00, 0x4B, 0x08, 0x68, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x07, 0xD2, 0x42, 0x00, 0x47, 0x09, 0x68, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x07, 0xF2, 0x42, 0x00, 0x43, 0x11, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x04, 0x90, 
0x80, 0x3F, 0x08, 0x68, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x0C, 0x90, 0x80, 0x3B, 0x36, 0x80, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x14, 0x90, 0x80, 0x37, 0x1D, 0xA0, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x02, 0x1C, 0x90, 0x80, 0x33, 0x19, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x24, 0x90, 
0x80, 0x2F, 0x08, 0x68, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x2C, 0x90, 0x80, 0x2B, 0x11, 0xA0, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x34, 0x90, 0x80, 0x27, 0x08, 0x68, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x02, 0x3C, 0x90, 0x80, 0x23, 0x15, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x44, 0x90,
0x80, 0x7C, 0x76, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x4C, 0x90, 0x80, 0x6C, 0x66, 0x80, 
0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x54, 0x90, 0x80, 0x5C, 0x21, 0xA0, 0x00, 0x00, 0x00, 0x01, 
0x68, 0x02, 0x5C, 0x90, 0x80, 0x4C, 0x25, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x64, 0x90, 
0x80, 0xF1, 0x5A, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x6C, 0x90, 0x80, 0xB1, 0x5A, 0x00, 0x00, 
0x00, 0x01, 0x68, 0x02, 0x74, 0x90, 0x81, 0xC5, 0x68, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x7C, 
0x90, 0x83, 0x1D, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x84, 0x90, 0x81, 0x42, 0x5A, 0x00, 
0x00, 0x00, 0x01, 0x68, 0x02, 0x8C, 0x90, 0x80, 0x91, 0xDA, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 
0x94, 0x90, 0x80, 0xD0, 0x86, 0x80, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0x9C, 0x90, 0x80, 0x44, 
0xDA, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0xA4, 0x90, 0x80, 0x54, 0x25, 0xA0, 0x00, 0x00, 0x00, 
0x01, 0x68, 0x02, 0xAC, 0x90, 0x80, 0x64, 0x25, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0xB4, 
0x90, 0x80, 0x74, 0x21, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0xBC, 0x90, 0x80, 0x21, 0x1D, 
0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0xC4, 0x90, 0x80, 0x25, 0x11, 0xA0, 0x00, 0x00, 0x00, 
0x01, 0x68, 0x02, 0xCC, 0x90, 0x80, 0x29, 0x15, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0xD4, 
0x90, 0x80, 0x2D, 0x08, 0x68, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0xDC, 0x90, 0x80, 0x31, 0x19, 
0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0xE4, 0x90, 0x80, 0x35, 0x11, 0xA0, 0x00, 0x00, 0x00, 
0x01, 0x68, 0x02, 0xEC, 0x90, 0x80, 0x39, 0x11, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0xF4, 
0x90, 0x80, 0x3D, 0x08, 0x68, 0x00, 0x00, 0x00, 0x01, 0x68, 0x02, 0xFC, 0x90, 0x80, 0x10, 0x45, 
0x68, 0x00, 0x00, 0x00, 0x01, 0x68, 0x03, 0x04, 0x90, 0x80, 0x11, 0x42, 0x5A, 0x00, 0x00, 0x00, 
0x01, 0x68, 0x03, 0x0C, 0x90, 0x80, 0x12, 0x47, 0x68, 0x00, 0x00, 0x00, 0x01, 0x68, 0x03, 0x14, 
0x90, 0x80, 0x13, 0x42, 0x1A, 0x00, 0x00, 0x00, 0x01, 0x68, 0x03, 0x1C, 0x90, 0x80, 0x14, 0x46, 
0x68, 0x00, 0x00, 0x00, 0x01, 0x68, 0x03, 0x24, 0x90, 0x80, 0x15, 0x45, 0x68, 0x00, 0x00, 0x00, 
0x01, 0x68, 0x03, 0x2C, 0x90, 0x80, 0x16, 0x42, 0x1A, 0x00, 0x00, 0x00, 0x01, 0x68, 0x03, 0x34, 
0x90, 0x80, 0x17, 0x47, 0x68, 0x00, 0x00, 0x00, 0x01, 0x68, 0x03, 0x3C, 0x90, 0x80, 0x18, 0x45, 
0x68, 0x00, 0x00, 0x00, 0x01, 0x68, 0x03, 0x44, 0x90, 0x80, 0x19, 0x45, 0x68, 0x00, 0x00, 0x01, 
0x06, 0x00, 0x11, 0x80, 0x19, 0x1D, 0xC4, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x15, 0x98, 
0x8B, 0x80, 0x00, 0x00, 0x01, 0x03, 0x00, 0x24, 0x20, 0x80, 0x00, 0x00, 0x01, 0x65, 0x88, 0x0D, 
0x80, 0x20, 0x01, 0x8C, 0x19, 0xE3, 0xE1, 0x0F, 0xD4, 0x50, 0x00, 0x21, 0x70, 0x10, 0xAD, 0x27, 
0x5A, 0xB1, 0xF9, 0xDE, 0xE0, 0x87, 0xF7, 0x02, 0x31, 0x49, 0xA2, 0xAF, 0x1C, 0x00, 0x0F, 0xC7, 
0x00, 0x03, 0xDF, 0xFC, 0xB8, 0x72, 0xEE, 0xE0, 0xAB, 0x60, 0x1F, 0x0F, 0x7F, 0x2E, 0x37, 0x9F, 
0x85, 0x59, 0x0B, 0x13, 0x96, 0x29, 0xFF, 0xE5, 0xFD, 0x6F, 0xDD, 0xF8, 0x09, 0x92, 0xA1, 0x9B, 
0x53, 0x0F, 0x52, 0x6D, 0x45, 0xFA, 0x65, 0xFF, 0x94, 0x30, 0xD6, 0xEF, 0xDF, 0x80, 0x21, 0xAD, 
0x28, 0xC3, 0x93, 0x2E, 0xDB, 0xAF, 0xED, 0xFF, 0xCB, 0x8F, 0x57, 0xBD, 0xEE, 0x5C, 0xC0, 0x4C, 
0x95, 0x19, 0xB5, 0x37, 0xC3, 0x4A, 0x67, 0x1F, 0xB0, 0x86, 0x97, 0x08, 0x4A, 0x18, 0x89, 0x17, 
0x51, 0x3A, 0x20, 0x81, 0x1E, 0xDA, 0x5F, 0x25, 0x7D, 0x8A, 0xB8, 0xDA, 0xA3, 0xE2, 0xFE, 0x03, 
0x8E, 0xFF, 0xA3, 0x90, 0xA0, 0xFF, 0x4A, 0xF5, 0x90, 0x11, 0x2A, 0x43, 0x41, 0xF1, 0x0F, 0x68, 
0xD1, 0x19, 0x2B, 0x7B, 0xA5, 0x3F, 0x6E, 0x4C, 0x4A, 0x6F, 0xDF, 0x55, 0xD2, 0x6A, 0x23, 0xBF, 
0x2F, 0xFD, 0xEF, 0xBF, 0xFF, 0x27, 0xC7, 0x00, 0x1D, 0xB8, 0x32, 0x1C, 0x2E, 0xC1, 0x17, 0xBB, 
0x4E, 0xD7, 0xEE, 0x36, 0xA6, 0x01, 0x0F, 0xD1, 0xFF, 0xE5, 0x3C, 0xF9, 0x86, 0xD6, 0xD9, 0xBD, 
0xFE, 0x79, 0xBF, 0xB4, 0xFF, 0xDF, 0xFB, 0xFE, 0xE8, 0x22, 0x3F, 0xB7, 0xD5, 0x7D, 0xD7, 0xAC, 
0xFB, 0x5D, 0xB6, 0xEE, 0x01, 0x12, 0xA4, 0x1A, 0x0F, 0x84, 0xFD, 0xB9, 0x32, 0x6F, 0x75, 0xBA, 
0xAB, 0x4B, 0xBE, 0xFF, 0xDF, 0x39, 0xFE, 0x01, 0x85, 0x9F, 0x26, 0x9F, 0xA2, 0x02, 0xDB, 0x38, 
0x6E, 0x66, 0xCB, 0xA9, 0x7C, 0xD1, 0xD7, 0xDF, 0x8E, 0xF9, 0xBB, 0xB3, 0x61, 0x0D, 0x3E, 0x36, 
0x28, 0x89, 0x67, 0x38, 0xD0, 0x6F, 0xC2, 0xDD, 0xF8, 0xCF, 0x72, 0xCB, 0x5A, 0xD6, 0xB6, 0xE0, 
0xC8, 0x34, 0x10, 0x00, 0x00, 0x01, 0x65, 0x1A, 0x20, 0x36, 0x00, 0x80, 0x06, 0x30, 0x67, 0xF8, 
0x7F, 0xC2, 0xD1, 0x40, 0x00, 0x87, 0xDF, 0x1C, 0x00, 0x04, 0x00, 0x63, 0x80, 0x01, 0xC9, 0xF0, 
0x64, 0x34, 0x0D, 0x3E, 0x0C, 0x86, 0x80, 0x7E, 0xFF, 0x49, 0x73, 0xD2, 0x68, 0xA1, 0x81, 0x13, 
0x19, 0x24, 0xDF, 0xF9, 0xFC, 0xDD, 0x71, 0x80, 0x00, 0x20, 0x03, 0x18, 0x00, 0x07, 0xA2, 0x30, 
0x64, 0x34, 0x1F, 0xA7, 0x23, 0x29, 0xE6, 0x7B, 0x7F, 0x3F, 0x2E, 0x27, 0x55, 0xAF, 0x5D, 0x60, 
0x0C, 0x8F, 0x6A, 0x6B, 0xEB, 0x73, 0xFE, 0xFF, 0x67, 0xBB, 0x42, 0x4C, 0xBA, 0x7D, 0x58, 0x57, 
0xDB, 0xC0, 0x94, 0x51, 0x32, 0x59, 0xBF, 0x5F, 0x5A, 0xDC, 0xA1, 0xC3, 0x3D, 0x24, 0x0A, 0xF1, 
0x97, 0x37, 0x11, 0xD3, 0xE1, 0x83, 0x0D, 0x72, 0x2A, 0xED, 0xD8, 0xD3, 0xF3, 0x68, 0x3A, 0xBD, 
0x3F, 0xE1, 0xB5, 0xBB, 0x6C, 0x31, 0x83, 0x1A, 0x9A, 0xF1, 0xAA, 0x7F, 0xD8, 0x81, 0x0A, 0xCD, 
0x5C, 0x5F, 0xA8, 0x42, 0xDF, 0xF6, 0x36, 0xEE, 0x4E, 0x3E, 0x85, 0xDC, 0x93, 0x33, 0xEB, 0x2A, 
0xE9, 0x6B, 0x32, 0x3E, 0xF7, 0x82, 0x1C, 0x9B, 0x25, 0xE0, 0xE3, 0xFC, 0x25, 0x33, 0xBA, 0x07, 
0xBF, 0xEA, 0xAD, 0xFF, 0xA7, 0xED, 0x3E, 0xF3, 0x7F, 0xCB, 0xE7, 0xCB, 0xFA, 0x67, 0x1F, 0xBF, 
0xFF, 0xC9, 0x18, 0x23, 0x8A, 0xE4, 0x7F, 0xE0, 0x42, 0x30, 0x8A, 0xB3, 0x88, 0xDD, 0xA5, 0xB9, 
0x22, 0x36, 0x88, 0xC0, 0xE9, 0xFA, 0x23, 0xFB, 0x78, 0x30, 0x6C, 0xDD, 0x73, 0xFD, 0xB4, 0x99, 
0xBC, 0xDA, 0x9B, 0xE1, 0x99, 0x77, 0xDD, 0x79, 0xFF, 0xB6, 0x0E, 0x12, 0x78, 0xDB, 0xB6, 0xF2, 
0x69, 0x62, 0x59, 0xD7, 0xFD, 0x4A, 0x33, 0xFF, 0xAA, 0x86, 0x7D, 0x6C, 0x51, 0x48, 0xCD, 0xB0, 
0xDF, 0x62, 0xAE, 0x67, 0x4C, 0x51, 0x42, 0xB7, 0xAC, 0xA9, 0x86, 0xE3, 0x8A, 0x40, 0x53, 0xCA, 
0xB7, 0x2E, 0xDF, 0x69, 0x37, 0xE6, 0xD4, 0xDF, 0xEB, 0x96, 0xF0, 0x3D, 0xDF, 0x2D, 0xF3, 0x26, 
0x01, 0xEE, 0xA2, 0x2C, 0xBB, 0x78, 0x59, 0xB3, 0x92, 0x18, 0xEE, 0x24, 0x83, 0x5F, 0xEF, 0x04, 
0x31, 0x4B, 0x9F, 0x5E, 0x12, 0x5D, 0x04, 0xF1, 0x04, 0x12, 0x23, 0x41, 0x20, 0x82, 0xFE, 0xDB, 
0x13, 0x0F, 0x8C, 0x72, 0x5C, 0x62, 0xFF, 0x5A, 0x33, 0xBF, 0x23, 0xDE, 0x93, 0x44, 0xA9, 0x40, 
0x3B, 0xCF, 0x37, 0xF1, 0xFA, 0xAE, 0x35, 0x3C, 0xEB, 0xA5, 0xF9, 0xB5, 0x7E, 0xFF, 0x5B, 0x04, 
0x35, 0xB3, 0xBE, 0x94, 0x9C, 0x68, 0x64, 0xFC, 0xE2, 0x80, 0xF9, 0x35, 0x2B, 0xA6, 0x8C, 0x9D, 
0xD3, 0xFE, 0x19, 0xAA, 0xDA, 0xBF, 0xBF, 0x1A, 0x55, 0x7C, 0x7A, 0x32, 0x2D, 0x57, 0x6F, 0x1B, 
0xC3, 0xD2, 0x55, 0xE6, 0x8F, 0x57, 0xAA, 0xA5, 0xF3, 0x34, 0x7F, 0xE6, 0x60
};


BOOL vH264BSParamConfig(VDEC_INFO_H264_BS_INIT_PRM_T *prH264BSInitPrm,VDEC_HAL_H264_BS_INIT_PRM_T *prH264HALBSPrm)
{
    if((prH264BSInitPrm == NULL) || (prH264HALBSPrm == NULL))
    {
        return FALSE;
    }

    prH264HALBSPrm->u4PredSa            = prH264BSInitPrm->u4PredSa;
    prH264HALBSPrm->u4VFifoEa           = prH264BSInitPrm->u4VFifoEa;
    prH264HALBSPrm->u4VFifoSa           = prH264BSInitPrm->u4VFifoSa;
    prH264HALBSPrm->u4VLDRdPtr          = prH264BSInitPrm->u4VLDRdPtr;
    prH264HALBSPrm->u4VLDWrPtr          = prH264BSInitPrm->u4VLDWrPtr;
    
    return TRUE;
}

BOOL vH264PRefParamConfig(VDEC_INFO_H264_P_REF_PRM_T *prPRefPicListInfo,VDEC_HAL_INFO_H264_P_REF_PRM_T *prPRefHalPrm)
{
    if((prPRefPicListInfo == NULL) || (prPRefHalPrm == NULL))
    {
        return FALSE;
    }

    prPRefHalPrm->ucFBufIdx             = prPRefPicListInfo->ucFBufIdx;
    prPRefHalPrm->i4BFldLongTermPicNum  = prPRefPicListInfo->i4BFldLongTermPicNum;
    prPRefHalPrm->i4BFldPicNum          = prPRefPicListInfo->i4BFldPicNum;
    prPRefHalPrm->i4BFldPOC             = prPRefPicListInfo->i4BFldPOC;
    prPRefHalPrm->i4PicNum              = prPRefPicListInfo->i4PicNum;
    prPRefHalPrm->i4LongTermPicNum      = prPRefPicListInfo->i4LongTermPicNum;
    prPRefHalPrm->i4TFldLongTermPicNum  = prPRefPicListInfo->i4TFldLongTermPicNum;
    prPRefHalPrm->i4TFldPicNum          = prPRefPicListInfo->i4TFldPicNum;
    prPRefHalPrm->i4TFldPOC             = prPRefPicListInfo->i4TFldPOC;
    prPRefHalPrm->u4BFldPara            = prPRefPicListInfo->u4BFldPara; 
    prPRefHalPrm->u4FBufYStartAddr      = prPRefPicListInfo->u4FBufYStartAddr;
    prPRefHalPrm->u4FBufCAddrOffset     = prPRefPicListInfo->u4FBufCAddrOffset;
    prPRefHalPrm->u4FBufMvStartAddr     = prPRefPicListInfo->u4FBufMvStartAddr;
    prPRefHalPrm->u4FBufInfo            = prPRefPicListInfo->u4FBufInfo;
    prPRefHalPrm->u4ListIdx             = prPRefPicListInfo->u4ListIdx;
    prPRefHalPrm->u4TFldPara            = prPRefPicListInfo->u4TFldPara;
    prPRefHalPrm->ucFBufIdx             = prPRefPicListInfo->ucFBufIdx;
    
    return TRUE;
}

BOOL vH264BRefParamConfig(VDEC_INFO_H264_B_REF_PRM_T *prBRefPicListInfo,VDEC_HAL_INFO_H264_B_REF_PRM_T *prBRefHalPrm)
{
    if((prBRefPicListInfo == NULL) || (prBRefHalPrm == NULL))
    {
       // MMLOG_TRACE(LOG_MOD_VDEC,TEXT("Error reference NULL pointer @ vH264BRefParamConfig!\n"));
        return FALSE;
    }

    prBRefHalPrm->ucFBufIdx                     = prBRefPicListInfo->ucFBufIdx;
    prBRefHalPrm->i4BFldLongTermPicNum          = prBRefPicListInfo->i4BFldLongTermPicNum;
    prBRefHalPrm->i4BFldPicNum                  = prBRefPicListInfo->i4BFldPicNum;
    prBRefHalPrm->i4BFldPOC                     = prBRefPicListInfo->i4BFldPOC;
    prBRefHalPrm->i4PicNum                      = prBRefPicListInfo->i4PicNum;
    prBRefHalPrm->i4LongTermPicNum              = prBRefPicListInfo->i4LongTermPicNum;
    prBRefHalPrm->i4TFldLongTermPicNum          = prBRefPicListInfo->i4TFldLongTermPicNum;
    prBRefHalPrm->i4TFldPicNum                  = prBRefPicListInfo->i4TFldPicNum;
    prBRefHalPrm->i4TFldPOC                     = prBRefPicListInfo->i4TFldPOC;
    prBRefHalPrm->u4BFldPara                    = prBRefPicListInfo->u4BFldPara;
    prBRefHalPrm->u4FBufYStartAddr              = prBRefPicListInfo->u4FBufYStartAddr;
    prBRefHalPrm->u4FBufCAddrOffset             = prBRefPicListInfo->u4FBufCAddrOffset;
    prBRefHalPrm->u4FBufMvStartAddr             = prBRefPicListInfo->u4FBufMvStartAddr;
    prBRefHalPrm->u4FBufInfo                    = prBRefPicListInfo->u4FBufInfo;
    prBRefHalPrm->u4ListIdx                     = prBRefPicListInfo->u4ListIdx;
    prBRefHalPrm->u4TFldPara                    = prBRefPicListInfo->u4TFldPara;
    prBRefHalPrm->u4ListIdx1                    = prBRefPicListInfo->u4ListIdx1;
    prBRefHalPrm->ucFBufIdx1                    = prBRefPicListInfo->ucFBufIdx1;
    prBRefHalPrm->u4FBufYStartAddr1             = prBRefPicListInfo->u4FBufYStartAddr1;
    prBRefHalPrm->u4FBufCAddrOffset1            = prBRefPicListInfo->u4FBufCAddrOffset1;
    prBRefHalPrm->u4FBufMvStartAddr1            = prBRefPicListInfo->u4FBufMvStartAddr1;
    prBRefHalPrm->i4LongTermPicNum1             = prBRefPicListInfo->i4LongTermPicNum1;
    prBRefHalPrm->i4PicNum1                     = prBRefPicListInfo->i4PicNum1;
    prBRefHalPrm->i4TFldPOC1                    = prBRefPicListInfo->i4TFldPOC1;
    prBRefHalPrm->i4BFldPOC1                    = prBRefPicListInfo->i4BFldPOC1;
    prBRefHalPrm->u4TFldPara1                   = prBRefPicListInfo->u4TFldPara1;
    prBRefHalPrm->u4BFldPara1                   = prBRefPicListInfo->u4BFldPara1;
    prBRefHalPrm->i4TFldPOC1                    = prBRefPicListInfo->i4TFldPOC1;
    prBRefHalPrm->i4TFldPOC1                    = prBRefPicListInfo->i4TFldPOC1;
    
    return TRUE;
}

BOOL vH264DecodeParamConfig(VDEC_INFO_DEC_PRM_T *prDecPrm,VDEC_HAL_H264_DEC_PRM_T *prH264HalPrm)
{
    VDEC_INFO_H264_DEC_PRM_T *prH264DecPrm ;
    
    if((prDecPrm == NULL) || (prH264HalPrm == NULL))
    {
       // MMLOG_TRACE(LOG_MOD_VDEC,TEXT("Error reference NULL pointer @ vH264DecodeParamConfig!\n"));
        return FALSE;
    }
    //[8563 try run] qianqian modify
    prH264DecPrm = (VDEC_INFO_H264_DEC_PRM_T *) &(prDecPrm->SpecDecPrm.rVDecH264DecPrm);
    prH264DecPrm->prCurrFBufInfo->u4CAddrOffset = prH264DecPrm->prCurrFBufInfo->u4DramPicSize;
    prH264DecPrm->prCurrFBufInfo->u4MvStartAddr = (prH264DecPrm->prCurrFBufInfo->u4YStartAddr + ((prH264DecPrm->prCurrFBufInfo->u4DramPicSize * 3) >>1));
    
    //prH264DecPrm = (VDEC_INFO_H264_DEC_PRM_T *)prDecPrm->prVDecCodecHalPrm;

    prH264HalPrm->ucAddrSwapMode    = prDecPrm->ucAddrSwapMode;
    prH264HalPrm->u4PicW            = prDecPrm->u4PicW;
    prH264HalPrm->u4PicH            = prDecPrm->u4PicH;
    prH264HalPrm->u4PicBW           = prDecPrm->u4PicBW;
    prH264HalPrm->ucECLevel         = prH264DecPrm->ucECLevel;
    prH264HalPrm->u4YStartAddr      = prH264DecPrm->prCurrFBufInfo->u4YStartAddr;
    prH264HalPrm->u4CAddrOffset     = prH264DecPrm->prCurrFBufInfo->u4CAddrOffset;
    prH264HalPrm->u4MvStartAddr     = prH264DecPrm->prCurrFBufInfo->u4MvStartAddr;
    prH264HalPrm->fgIsFrmPic        = prH264DecPrm->fgIsFrmPic;
    prH264HalPrm->ucPicStruct       = prDecPrm->ucPicStruct;
    prH264HalPrm->ucNalRefIdc       = prH264DecPrm->ucNalRefIdc;
    prH264HalPrm->fgIsIDRPic        = prH264DecPrm->fgIsIDRPic;

    //slice hdr2 info
    prH264HalPrm->rSliceHALParam.i4SliceQpDelta                 = prH264DecPrm->prSliceHdr->i4SliceQpDelta;
    prH264HalPrm->rSliceHALParam.u4DisableDeblockingFilterIdc   = prH264DecPrm->prSliceHdr->u4DisableDeblockingFilterIdc;
    prH264HalPrm->rSliceHALParam.i4SliceAlphaC0OffsetDiv2       = prH264DecPrm->prSliceHdr->i4SliceAlphaC0OffsetDiv2;
    prH264HalPrm->rSliceHALParam.i4SliceBetaOffsetDiv2          = prH264DecPrm->prSliceHdr->i4SliceBetaOffsetDiv2;
    prH264HalPrm->rSliceHALParam.u4CabacInitIdc                 = prH264DecPrm->prSliceHdr->u4CabacInitIdc;
#if 0
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--ucAddrSwapMode %d"), prH264HalPrm->ucAddrSwapMode);
    
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--u4PicW %d"), prH264HalPrm->u4PicW);
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--u4PicH %d"), prH264HalPrm->u4PicH);
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--u4PicBW %d"), prH264HalPrm->u4PicBW);
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--ucECLevel %d"), prH264HalPrm->ucECLevel);
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--fgIsFrmPic %d"), prH264HalPrm->fgIsFrmPic);
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--ucPicStruct %d"), prH264HalPrm->ucPicStruct);
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--ucNalRefIdc %d"), prH264HalPrm->ucNalRefIdc);
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--fgIsIDRPic %d"), prH264HalPrm->fgIsIDRPic);
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--i4SliceQpDelta %d"), prH264HalPrm->rSliceHALParam.i4SliceQpDelta );
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--u4DisableDeblockingFilterIdc %d"), prH264HalPrm->rSliceHALParam.u4DisableDeblockingFilterIdc );
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--i4SliceAlphaC0OffsetDiv2 %d"), prH264HalPrm->rSliceHALParam.i4SliceAlphaC0OffsetDiv2 );
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--i4SliceBetaOffsetDiv2 %d"), prH264HalPrm->rSliceHALParam.i4SliceBetaOffsetDiv2 );
    MMLOG_DEBUG(LOG_MOD_VDEC, TEXT("H264--u4CabacInitIdc %d"), prH264HalPrm->rSliceHALParam.u4CabacInitIdc );
#endif
    return TRUE;
}


UCHAR bGetPicRefType(UINT32 u4InstID, UCHAR ucPicStruct)
{
    return _ptCurrFBufInfo[u4InstID]->ucFBufRefType;
}

BOOL fgH264_SLT_CheckCRCResult(UINT32 u4InstID, UINT32 u4ProcessFrameCnt)
    {
    UINT32 u4HWCRC_Y0, u4HWCRC_Y1, u4HWCRC_Y2, u4HWCRC_Y3;
    UINT32 u4HWCRC_C0, u4HWCRC_C1, u4HWCRC_C2, u4HWCRC_C3;
    UINT32 u4CRCValueY0, u4CRCValueY1, u4CRCValueY2, u4CRCValueY3;
    UINT32 u4CRCValueC0, u4CRCValueC1, u4CRCValueC2, u4CRCValueC3;
    UINT32 u4CRCTmp3, u4CRCTmp2, u4CRCTmp1, u4CRCTmp0;
    BOOL fgDecErr = FALSE;
    BOOL fgCRCPass = TRUE;
    
    if (u4ProcessFrameCnt == 0) {//80x96
        u4CRCValueY0 = 0xa9454ada;
        u4CRCValueY1 = 0xe7b1fe13;
        u4CRCValueY2 = 0x2a53b472;
        u4CRCValueY3 = 0x305250f4;
        u4CRCValueC0 = 0xf42753ab;
        u4CRCValueC1 = 0x7b2f81db;
        u4CRCValueC2 = 0x8ef827c5;
        u4CRCValueC3 = 0x5d9aca76;
    }
    u4HWCRC_Y0 = u4VDecReadCRC(u4InstID, VDEC_CRC_Y_CHKSUM0);
    u4HWCRC_Y1 = u4VDecReadCRC(u4InstID, VDEC_CRC_Y_CHKSUM1);
    u4HWCRC_Y2 = u4VDecReadCRC(u4InstID, VDEC_CRC_Y_CHKSUM2);
    u4HWCRC_Y3 = u4VDecReadCRC(u4InstID, VDEC_CRC_Y_CHKSUM3);

    u4HWCRC_C0 = u4VDecReadCRC(u4InstID, VDEC_CRC_C_CHKSUM0);
    u4HWCRC_C1 = u4VDecReadCRC(u4InstID, VDEC_CRC_C_CHKSUM1);
    u4HWCRC_C2 = u4VDecReadCRC(u4InstID, VDEC_CRC_C_CHKSUM2);
    u4HWCRC_C3 = u4VDecReadCRC(u4InstID, VDEC_CRC_C_CHKSUM3);

    if( (u4HWCRC_Y0 != u4CRCValueY0) 
    	|| (u4HWCRC_Y1 != u4CRCValueY1)
    	|| (u4HWCRC_Y2 != u4CRCValueY2)
    	|| (u4HWCRC_Y3 != u4CRCValueY3)
    	|| (u4HWCRC_C0 != u4CRCValueC0)
    	|| (u4HWCRC_C1 != u4CRCValueC1)
    	|| (u4HWCRC_C2 != u4CRCValueC2)
    	|| (u4HWCRC_C3 != u4CRCValueC3)
      )
    {
       fgDecErr = TRUE;
       fgCRCPass = FALSE;
       #if 0
       Printf("[AVC_SFT] u4HWCRC_Y0:0x%lx, u4HWCRC_Y1:0x%lx, u4HWCRC_Y2:0x%lx, u4HWCRC_Y3:0x%lx,u4HWCRC_C0:0x%lx, u4HWCRC_C1:0x%lx,u4HWCRC_C2:0x%lx, u4HWCRC_C3:0x%lx\n",
        u4HWCRC_Y0, u4HWCRC_Y1, u4HWCRC_Y2, u4HWCRC_Y3, u4HWCRC_C0, u4HWCRC_C1, u4HWCRC_C2,u4HWCRC_C3);
       Printf("[AVC_SFT] u4CRCValueY0:0x%lx, u4CRCValueY1:0x%lx, u4CRCValueY2:0x%lx,u4CRCValueY3:0x%lx, u4CRCValueC0:0x%lx,u4CRCValueC1:0x%lx, u4CRCValueC2:0x%lx,u4CRCValueC3:0x%lx\n",
        u4CRCValueY0, u4CRCValueY1, u4CRCValueY2, u4CRCValueY3, u4CRCValueC0, u4CRCValueC1, u4CRCValueC2,u4CRCValueC3);
        #endif
       return FALSE;
    }
    return fgCRCPass;
}

VOID vAVC_SLT_SetCurrFBufIdx(UINT32 u4InstID, UINT32 u4DecFBufIdx)
{
    _ptCurrFBufInfo[u4InstID] = &_ptFBufInfo[u4InstID][u4DecFBufIdx];
    _pucAVCDecWorkBuf[u4InstID] = (UCHAR *)(_ptCurrFBufInfo[u4InstID]->u4Addr);
    //Printf(" u4DecFBufIdx %d, _pucAVCDecWorkBuf[u4InstID] %d", u4DecFBufIdx, (UINT32)_pucAVCDecWorkBuf[u4InstID]);
}

VOID vAVC_SLT_ClrFBufInfo(UINT32 u4InstID, UINT32 u4FBufIdx)
{
  _ptFBufInfo[u4InstID][u4FBufIdx].fgNonExisting = FALSE;    
  _ptFBufInfo[u4InstID][u4FBufIdx].eH264DpbStatus = H264_DPB_STATUS_EMPTY;
  _ptFBufInfo[u4InstID][u4FBufIdx].ucFBufStatus = NO_PIC;    
  
  _ptFBufInfo[u4InstID][u4FBufIdx].ucBFldRefType = NREF_PIC;    
  _ptFBufInfo[u4InstID][u4FBufIdx].ucFBufRefType = NREF_PIC;
  _ptFBufInfo[u4InstID][u4FBufIdx].ucTFldRefType = NREF_PIC;
  
  _ptFBufInfo[u4InstID][u4FBufIdx].u4FrameNum = 0xffffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4FrameNumWrap = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4PicNum = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4TFldPicNum = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4BFldPicNum = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].u4LongTermFrameIdx = 0xffffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].u4TFldLongTermFrameIdx = 0xffffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].u4BFldLongTermFrameIdx = 0xffffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4LongTermPicNum = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4TFldLongTermPicNum = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4BFldLongTermPicNum = 0xefffffff;    

  _ptFBufInfo[u4InstID][u4FBufIdx].ucFBufStatus = 0;
  _ptFBufInfo[u4InstID][u4FBufIdx].u4TFldPara = 0;
  _ptFBufInfo[u4InstID][u4FBufIdx].u4BFldPara = 0;
  
  
  _ptFBufInfo[u4InstID][u4FBufIdx].i4POC = 0x7fffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4TFldPOC = 0x7fffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4BFldPOC = 0x7fffffff;
}

void vAVC_SLT_AllocateFBuf(UINT32 u4InstID, VDEC_INFO_DEC_PRM_T *tVDecPrm, BOOL fgFillCurrFBuf)
{
  INT32 i;
  INT32 iMinPOC;
  UINT32 u4MinPOCFBufIdx = 0;
  
  // Check if DPB full
  iMinPOC = 0x7fffffff;
  for(i=0; i<_tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum; i++)
  {
    if(_ptFBufInfo[u4InstID][i].ucFBufStatus == NO_PIC)
    {
      iMinPOC = 0x7fffffff;
      u4MinPOCFBufIdx = i;        
      break;
    }
    // miew: need to take care of field empty
    else if((iMinPOC > _ptFBufInfo[u4InstID][i].i4POC) && fgIsNonRefFBuf(u4InstID, i))
    {
      iMinPOC = _ptFBufInfo[u4InstID][i].i4POC;
      u4MinPOCFBufIdx = i;
    }
  }  
  // No empty DPB, 1 FBuf output
  if(_ptFBufInfo[u4InstID][u4MinPOCFBufIdx].ucFBufStatus != NO_PIC)
  {
    vAVC_SLT_ClrFBufInfo(u4InstID, u4MinPOCFBufIdx);
  }
  _tVDecPrm[u4InstID].ucDecFBufIdx = u4MinPOCFBufIdx;
  // Only new alloc needs to update current fbuf idx
  vAVC_SLT_SetCurrFBufIdx(u4InstID, _tVDecPrm[u4InstID].ucDecFBufIdx);
}

VOID vAVC_SLT_PartitionDPB(UINT32 u4InstID) 
{
  INT32 i;
  UINT32 u4DramPicSize;
  UINT32 u4DramPicArea;
  
  // Real pic size w=64x, h=32x  
  u4DramPicSize = ((((_tVDecPrm[u4InstID].u4PicW + 63) >> 6) * ((_tVDecPrm[u4InstID].u4PicH + 31) >> 5)) <<11);
  //For Swap mode
  // 1 pic area = Y + CbCr +MV
  u4DramPicArea = ((((u4DramPicSize * 7) >> 2) + 4095) >> 12)<< 12;    
  _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum = AVC_SLT_DPB_SZ / u4DramPicArea;

  if(_tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum > 17)
  {
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum = 17;
  }
#ifdef DEBUG_VDEC
  Printf("[AVC_SFT]vAVC_SLT_PartitionDPB ucMaxFBufNum %d\n", _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum);
#endif

  if (_tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum * u4DramPicArea > AVC_SLT_DPB_SZ)
  {
       Printf("[AVC_SFT]vAVC_SLT_PartitionDPB ERROR AVC_SLT_DPB_SZ 0x%x, MAXDramArea 0x%x\n", AVC_SLT_DPB_SZ, _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum * u4DramPicArea);
  }
  
  for(i=0; i<_tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum; i++)
  {
    _ptFBufInfo[u4InstID][i].u4W = _tVDecPrm[u4InstID].u4PicW;
    _ptFBufInfo[u4InstID][i].u4H = _tVDecPrm[u4InstID].u4PicH;    
    _ptFBufInfo[u4InstID][i].u4DramPicSize = u4DramPicSize;
    _ptFBufInfo[u4InstID][i].u4DramPicArea = u4DramPicArea;    
    _ptFBufInfo[u4InstID][i].u4Addr = _pu4AVCDPBBUF + (i * u4DramPicArea);
     {
        //Printf("H264 Use Large B MV Buffer\n");
        _ptFBufInfo[u4InstID][i].u4MvStartAddr = _ptFBufInfo[u4InstID][i].u4Addr + ((u4DramPicSize * 3) >> 1);
     }
  }
  // current reset to 0 when DPB partition.
  _ptCurrFBufInfo[u4InstID] = &_ptFBufInfo[u4InstID][0];
}

UCHAR ucAVC_SLT_VDecGetMinPOCFBuf(UINT32 u4InstID, BOOL fgWithEmpty)
{
    UINT32 u4Idx;
    UINT32 u4MinPOCFBufIdx;
    INT32 i4MinPOC;

    i4MinPOC = 0x7fffffff;
    u4MinPOCFBufIdx = 0xFF;
    for(u4Idx=0; u4Idx < _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum; u4Idx++)
    {
        if((_ptFBufInfo[u4InstID][u4Idx].eH264DpbStatus == H264_DPB_STATUS_OUTPUTTED)
            && fgIsNonRefFBuf(u4InstID, u4Idx))
        {
            vAVC_SLT_ClrFBufInfo(u4InstID, u4Idx);

        }
        if(fgWithEmpty)
        {
		if((i4MinPOC != 0x80000001) && (_ptFBufInfo[u4InstID][u4Idx].eH264DpbStatus == H264_DPB_STATUS_EMPTY))
	    	{
			i4MinPOC = 0x80000001;
			u4MinPOCFBufIdx = u4Idx;
	    	}
		else if((i4MinPOC > _ptFBufInfo[u4InstID][u4Idx].i4POC) && (_ptFBufInfo[u4InstID][u4Idx].eH264DpbStatus == H264_DPB_STATUS_DECODED))
		{
	            i4MinPOC = _ptFBufInfo[u4InstID][u4Idx].i4POC;
	            u4MinPOCFBufIdx = u4Idx;
		}
            //break;
        }
        else // Flush
    	{
	        // Need to take care of field empty
	        if(i4MinPOC > _ptFBufInfo[u4InstID][u4Idx].i4POC)
	        {
	            i4MinPOC = _ptFBufInfo[u4InstID][u4Idx].i4POC;
	            u4MinPOCFBufIdx = u4Idx;
	        }
    	}
    }  

    return u4MinPOCFBufIdx;
}


VOID vAVC_SLT_FlushDPB(UINT32 u4InstID, BOOL fgWithOutput)
{
  UINT32 u4MinPOCFBufIdx;

  do
  {
        u4MinPOCFBufIdx = ucAVC_SLT_VDecGetMinPOCFBuf(u4InstID, FALSE);
        if(u4MinPOCFBufIdx != 0xFF)
        {
            if(fgWithOutput
                && _ptFBufInfo[u4InstID][u4MinPOCFBufIdx].eH264DpbStatus == H264_DPB_STATUS_DECODED)
            {
                //_ptFBufInfo[u4MinPOCFBufIdx].eH264DpbStatus = H264_DPB_STATUS_OUTPUTTED;
            }
            // Force set outputted
            _ptFBufInfo[u4InstID][u4MinPOCFBufIdx].eH264DpbStatus = H264_DPB_STATUS_OUTPUTTED;
            _ptFBufInfo[u4InstID][u4MinPOCFBufIdx].ucFBufRefType = NREF_PIC;
            _ptFBufInfo[u4InstID][u4MinPOCFBufIdx].ucTFldRefType = NREF_PIC;
            _ptFBufInfo[u4InstID][u4MinPOCFBufIdx].ucBFldRefType = NREF_PIC;
        }
  }while(u4MinPOCFBufIdx != 0xff);

}

VOID vAVC_SLT_VDecSetCurrPOC(UINT32 u4InstID)
{
  INT32 iPrevPOCMsb;
  INT32 iPrevPOCLsb;    
  INT32 iMaxPicOrderCntLsb;
  INT32 iPrevFrameNumOffset;
  INT32 iAbsFrameNum;
  INT32 iPicOrderCntCycleCnt = 0;
  INT32 iFrameNumInPicOrderCntCycle = 0;
  INT32 iExpectedDeltaPerPicOrderCnt = 0;
  INT32 iExpectedDeltaPerPicOrderCntCycle;
  UINT32 i;
  INT32  j;
  INT32 iTemp;
  
  VDEC_INFO_DEC_PRM_T *tVDecPrm;

  tVDecPrm = &_tVDecPrm[u4InstID];
  iMaxPicOrderCntLsb = 1 << (tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4Log2MaxPicOrderCntLsbMinus4 + 4);
  
  switch(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4PicOrderCntType)
  {
    case 0:
      if(fgSLTIsIDRPic(u4InstID))
      {
        iPrevPOCMsb = 0;
        iPrevPOCLsb = 0;        
      }
      else
      {
        if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.fgLastMmco5)
        {
          if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.ucLastPicStruct != BOTTOM_FIELD)
          {
            iPrevPOCMsb = 0;
            iPrevPOCLsb = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.i4LastRefTFldPOC;        
          }
          else
          {
            iPrevPOCMsb = 0;
            iPrevPOCLsb = 0;        
          }
        }
        else
        {
            iPrevPOCMsb = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.i4LastRefPOCMsb;        
            iPrevPOCLsb = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.i4LastRefPOCLsb;        
        }
      }

      // Calculate POCMsb
      if((tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4PicOrderCntLsb <  iPrevPOCLsb) && 
         ((iPrevPOCLsb - tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4PicOrderCntLsb) >= (iMaxPicOrderCntLsb >> 1)))
      {
        tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4PicOrderCntMsb = iPrevPOCMsb + iMaxPicOrderCntLsb;
      }
      else if((tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4PicOrderCntLsb >  iPrevPOCLsb) && 
         ((tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4PicOrderCntLsb - iPrevPOCLsb) > (iMaxPicOrderCntLsb >> 1)))
      {
        tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4PicOrderCntMsb = iPrevPOCMsb - iMaxPicOrderCntLsb;
      }
      else
      {
        tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4PicOrderCntMsb = iPrevPOCMsb;
      }

      if((!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgFieldPicFlag) || (!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgBottomFieldFlag))
      {
        _ptCurrFBufInfo[u4InstID]->i4TFldPOC = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4PicOrderCntMsb + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4PicOrderCntLsb;
      }

      if((!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgFieldPicFlag))
      {
        _ptCurrFBufInfo[u4InstID]->i4BFldPOC = _ptCurrFBufInfo[u4InstID]->i4TFldPOC + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCntBottom;
      }
      else if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgBottomFieldFlag)
      {
        _ptCurrFBufInfo[u4InstID]->i4BFldPOC = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4PicOrderCntMsb + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4PicOrderCntLsb;
      }
      break;
    case 1:
      if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.fgLastMmco5)
      {
        iPrevFrameNumOffset = 0;
      }
      else
      {
        iPrevFrameNumOffset = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.i4LastFrameNumOffset;
      }
      if(fgSLTIsIDRPic(u4InstID))
      {
        tVDecPrm->SpecDecPrm.rVDecH264DecPrm.i4FrmNumOffset = 0;
      }
      else if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastFrameNum > tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4FrameNum)
      {
        tVDecPrm->SpecDecPrm.rVDecH264DecPrm.i4FrmNumOffset = iPrevFrameNumOffset + (INT32)tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4MaxFrameNum;
      }
      else
      {
        tVDecPrm->SpecDecPrm.rVDecH264DecPrm.i4FrmNumOffset = iPrevFrameNumOffset;
      }

      if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4NumRefFramesInPicOrderCntCycle != 0)
      {
        iAbsFrameNum = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.i4FrmNumOffset + (INT32)tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4FrameNum;
      }
      else
      {
        iAbsFrameNum = 0; 
      }
      
      if(_u4NalRefIdc[u4InstID] == 0 && iAbsFrameNum > 0)
      {
        iAbsFrameNum --;
      }

      if(iAbsFrameNum > 0)
      {
        iPicOrderCntCycleCnt = (iAbsFrameNum - 1)/tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4NumRefFramesInPicOrderCntCycle;
        iFrameNumInPicOrderCntCycle = (iAbsFrameNum - 1)%tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4NumRefFramesInPicOrderCntCycle;        
      }

      iExpectedDeltaPerPicOrderCntCycle = 0;
      for(i=0; i<tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4NumRefFramesInPicOrderCntCycle; i++)
      {
        iExpectedDeltaPerPicOrderCntCycle += tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->i4OffsetForRefFrame[i];
      }
      if(iAbsFrameNum > 0)
      {
        iExpectedDeltaPerPicOrderCnt = (INT32)iPicOrderCntCycleCnt * iExpectedDeltaPerPicOrderCntCycle;
        for(j=0; j<=iFrameNumInPicOrderCntCycle; j++)
        {
          iExpectedDeltaPerPicOrderCnt = iExpectedDeltaPerPicOrderCnt + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->i4OffsetForRefFrame[i];
        }
      }
      else
      {
        iExpectedDeltaPerPicOrderCnt = 0;
      }
      if(_u4NalRefIdc[u4InstID] == 0)
      {
        iExpectedDeltaPerPicOrderCnt = iExpectedDeltaPerPicOrderCnt + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->i4OffsetForNonRefPic;
      }
      if(!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgFieldPicFlag)
      {
        _ptCurrFBufInfo[u4InstID]->i4TFldPOC = iExpectedDeltaPerPicOrderCnt + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[0];
        iTemp = _ptCurrFBufInfo[u4InstID]->i4TFldPOC + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->i4OffsetForTopToBottomField;
        _ptCurrFBufInfo[u4InstID]->i4BFldPOC = iTemp + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[1];
        _ptCurrFBufInfo[u4InstID]->i4BFldPOC = _ptCurrFBufInfo[u4InstID]->i4TFldPOC + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->i4OffsetForTopToBottomField + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[1];
      }
      else if(!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgBottomFieldFlag)
      {
        _ptCurrFBufInfo[u4InstID]->i4TFldPOC = iExpectedDeltaPerPicOrderCnt + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[0];
      }
      else
      {
        _ptCurrFBufInfo[u4InstID]->i4BFldPOC = iExpectedDeltaPerPicOrderCnt + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->i4OffsetForTopToBottomField + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[0];      
      }
      break;
    case 2:
      if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.fgLastMmco5)
      {
        iPrevFrameNumOffset = 0;
      }
      else
      {
        iPrevFrameNumOffset = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.i4LastFrameNumOffset;
      }      
      if(fgSLTIsIDRPic(u4InstID))
      {
        tVDecPrm->SpecDecPrm.rVDecH264DecPrm.i4FrmNumOffset = 0;
      }
      else if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastFrameNum > tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4FrameNum)
      {
        tVDecPrm->SpecDecPrm.rVDecH264DecPrm.i4FrmNumOffset = iPrevFrameNumOffset + (INT32)tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4MaxFrameNum;
      }
      else
      {
        tVDecPrm->SpecDecPrm.rVDecH264DecPrm.i4FrmNumOffset = iPrevFrameNumOffset;
      }
      if(fgSLTIsIDRPic(u4InstID))
      {
        // Use iAbsFrameNum as tempPicOrderCnt
        iAbsFrameNum = 0;
      }
      else if(_u4NalRefIdc[u4InstID] == 0)
      {
        iAbsFrameNum = ((tVDecPrm->SpecDecPrm.rVDecH264DecPrm.i4FrmNumOffset + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4FrameNum)  << 1) + 1;
      }
      else
      {
        iAbsFrameNum = ((tVDecPrm->SpecDecPrm.rVDecH264DecPrm.i4FrmNumOffset + tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4FrameNum)  << 1);        
      }
      
      if(!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgFieldPicFlag)
      {
        _ptCurrFBufInfo[u4InstID]->i4TFldPOC = iAbsFrameNum;
        _ptCurrFBufInfo[u4InstID]->i4BFldPOC = iAbsFrameNum;
      }
      else if(!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgBottomFieldFlag)
      {
        _ptCurrFBufInfo[u4InstID]->i4TFldPOC = iAbsFrameNum;
      }
      else
      {
        _ptCurrFBufInfo[u4InstID]->i4BFldPOC = iAbsFrameNum;
      }     
      break;
    default:
      break;
  }
  _ptCurrFBufInfo[u4InstID]->i4POC = (_ptCurrFBufInfo[u4InstID]->i4TFldPOC < _ptCurrFBufInfo[u4InstID]->i4BFldPOC)? _ptCurrFBufInfo[u4InstID]->i4TFldPOC : _ptCurrFBufInfo[u4InstID]->i4BFldPOC;
}


VOID vAVC_SLT_SlidingWindowProce(UINT32 u4InstID)
{
  INT32 i;
  INT32 iMinFrameNumWrap;
  INT32 i4FrameNumWrap;
  UINT32 u4NumShortTerm;
  UINT32 u4NumLongTerm;
  UINT32 u4MinFBufIdx = 0;

  // If the curr pic is the 2nd field, follow the 1st field's ref info
  if((_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME) && (_tVDecPrm[u4InstID].ucPicStruct == BOTTOM_FIELD) && (_ptCurrFBufInfo[u4InstID]->ucTFldRefType == SREF_PIC))
  {
    _ptCurrFBufInfo[u4InstID]->ucBFldRefType = SREF_PIC;
  }
  else if((_ptCurrFBufInfo[u4InstID]->ucFBufStatus == FRAME) && (_tVDecPrm[u4InstID].ucPicStruct == TOP_FIELD) && (_ptCurrFBufInfo[u4InstID]->ucBFldRefType == SREF_PIC))
  {
    _ptCurrFBufInfo[u4InstID]->ucTFldRefType = SREF_PIC;    
  }
  else
  {
    i = 0;
    iMinFrameNumWrap = 0xfffffff;
    u4NumShortTerm = 0;
    u4NumLongTerm = 0;  
    // Remove 1 SREF pic for a new ref pic
    for(i=0; i<_tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum; i++)
    {
      if((_ptFBufInfo[u4InstID][i].ucTFldRefType == SREF_PIC) || (_ptFBufInfo[u4InstID][i].ucBFldRefType == SREF_PIC))
      {
        i4FrameNumWrap = (_ptFBufInfo[u4InstID][i].u4FrameNum > _ptCurrFBufInfo[u4InstID]->u4FrameNum)? (_ptFBufInfo[u4InstID][i].u4FrameNum -_tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prSPS->u4MaxFrameNum) : _ptFBufInfo[u4InstID][i].u4FrameNum;
        if(iMinFrameNumWrap > i4FrameNumWrap)
        {
          iMinFrameNumWrap =  i4FrameNumWrap;
          u4MinFBufIdx = i;        
        }
        u4NumShortTerm ++;     
      }
      if((_ptFBufInfo[u4InstID][i].ucTFldRefType == LREF_PIC) || (_ptFBufInfo[u4InstID][i].ucBFldRefType == LREF_PIC))
      {
        u4NumLongTerm ++;
      }
    }
    // Since current pic should be ref pic, the condition should be modified as "larger" only
    // but the current one not set as ref pic at this time,
    if((u4NumShortTerm + u4NumLongTerm) >= ((_tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prSPS->u4NumRefFrames > 0)? _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prSPS->u4NumRefFrames : 1))
    {
      // Remove the smallet FrameNumWrap item
      _ptFBufInfo[u4InstID][u4MinFBufIdx].ucFBufRefType = NREF_PIC;
      _ptFBufInfo[u4InstID][u4MinFBufIdx].ucTFldRefType = NREF_PIC;
      _ptFBufInfo[u4InstID][u4MinFBufIdx].ucBFldRefType = NREF_PIC;    
    }
  }
}

VOID vAVC_SLT_FillFrameNumGap(UINT32 u4InstID, VDEC_INFO_DEC_PRM_T *tVDecPrm)
{
  UINT32 u4CurrFrameNum;
  UINT32 u4UnusedShortTermFrameNum;
  INT32 tmp1 = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[0];
  INT32 tmp2 = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[1];
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[0] = 0;
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[1] = 0;

  u4UnusedShortTermFrameNum = (tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastFrameNum + 1) % tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4MaxFrameNum;
  u4CurrFrameNum = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4FrameNum;

  if((tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4FrameNum != tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastFrameNum) 
      && (u4CurrFrameNum != u4UnusedShortTermFrameNum))
  {
    while (u4CurrFrameNum != u4UnusedShortTermFrameNum)
    {
      // Create a new frame pic
      vAVC_SLT_AllocateFBuf(u4InstID, tVDecPrm, FALSE);
      _ptCurrFBufInfo[u4InstID]->ucFBufStatus = FRAME;
      _ptCurrFBufInfo[u4InstID]->i4PicNum = u4UnusedShortTermFrameNum;
      _ptCurrFBufInfo[u4InstID]->u4FrameNum = u4UnusedShortTermFrameNum;
      _ptCurrFBufInfo[u4InstID]->fgNonExisting = TRUE;
      
      tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgAdaptiveRefPicMarkingModeFlag = 0;

      if (tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4PicOrderCntType == 0)
      {
        _ptCurrFBufInfo[u4InstID]->i4POC = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.i4LastPOC;      
        _ptCurrFBufInfo[u4InstID]->i4TFldPOC = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.i4LastTFldPOC;
        _ptCurrFBufInfo[u4InstID]->i4BFldPOC = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.i4LastBFldPOC;
      }
      else
      {
        vAVC_SLT_VDecSetCurrPOC(u4InstID);
      }
      // Check if out of the Ref Frames
      vAVC_SLT_SlidingWindowProce(u4InstID);
      _ptCurrFBufInfo[u4InstID]->ucFBufRefType = SREF_PIC;
      _ptCurrFBufInfo[u4InstID]->ucTFldRefType = SREF_PIC;
      _ptCurrFBufInfo[u4InstID]->ucBFldRefType = SREF_PIC;        
      tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastFrameNum = u4UnusedShortTermFrameNum;
      u4UnusedShortTermFrameNum = (u4UnusedShortTermFrameNum + 1) % tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4MaxFrameNum;
    }    
  }

  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[0] = tmp1;
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[1] = tmp2;
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4FrameNum = u4CurrFrameNum;
}

VOID vAVC_SLT_PrepareFBufInfo(UINT32 u4InstID, VDEC_INFO_DEC_PRM_T *tVDecPrm)
{
  tVDecPrm->u4PicW = (tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4PicWidthInMbsMinus1 + 1) << 4;
  tVDecPrm->u4PicH = (2 - tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgFrameMbsOnlyFlag)*(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4PicHeightInMapUnitsMinus1 + 1) << 4; //32x
  tVDecPrm->u4PicBW = tVDecPrm->u4PicW;
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.u4RealPicH = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4PicHeightInMapUnitsMinus1 << 4;  // original real size
  //Printf("tVDecPrm->u4PicW %d u4PicW %d", tVDecPrm->u4PicW, tVDecPrm->u4PicH);
  if((tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastPicW != 
    tVDecPrm->u4PicW) || (tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastPicH != tVDecPrm->u4PicH))
  {
    vAVC_SLT_PartitionDPB(u4InstID);      
  }

  if(fgSLTIsIDRPic(u4InstID))
  {
    tVDecPrm->SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastFrameNum = 0xffffffff;
    //vFlushDPB(u4InstID, tVerMpvDecPrm, FALSE);
    if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgNoOutputOfPriorPicsFlag)
    {
        vAVC_SLT_FlushDPB(u4InstID, FALSE);
    }
    else
    {
        vAVC_SLT_FlushDPB(u4InstID, TRUE);
    }
  } 
  
  if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgGapsInFrameNumValueAllowedFlag)
  {
    vAVC_SLT_FillFrameNumGap(u4InstID, tVDecPrm);
  }

  // Find a empty fbuf 
  if((_ptCurrFBufInfo[u4InstID]->ucFBufStatus == NO_PIC)
      || (_ptCurrFBufInfo[u4InstID]->ucFBufStatus & tVDecPrm->ucPicStruct))
  {
    vAVC_SLT_AllocateFBuf(u4InstID, tVDecPrm, TRUE);   
  }
  
  _ptCurrFBufInfo[u4InstID]->ucFBufStatus |= _tVDecPrm[u4InstID].ucPicStruct;

  if(tVDecPrm->ucPicStruct & TOP_FIELD)
  {
    _ptCurrFBufInfo[u4InstID]->u4TFldPara = ((fgSLTIsFrmPic(u4InstID)? 0 : 1) << 19) + ((fgSLTIsFrmPic(u4InstID) && tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgMbAdaptiveFrameFieldFlag) << 18);
  }
  if(tVDecPrm->ucPicStruct & BOTTOM_FIELD)
  {
    _ptCurrFBufInfo[u4InstID]->u4BFldPara = ((fgSLTIsFrmPic(u4InstID)? 0 : 1) << 19) + ((fgSLTIsFrmPic(u4InstID) && tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgMbAdaptiveFrameFieldFlag) << 18);
  }
  
}

BOOL fgAVC_SLT_ChkRefInfo(UINT32 u4InstID, UINT32 u4FBufIdx, UINT32 u4RefType)
{
  if(fgSLTIsFrmPic(u4InstID))
  {
    // According to spec 8.2.4.2.1 
    // NOTE: A non-pared reference fiedl is not used for inter prediction for decoding a frame.
    if((_ptFBufInfo[u4InstID][u4FBufIdx].ucTFldRefType == u4RefType) && (_ptFBufInfo[u4InstID][u4FBufIdx].ucBFldRefType == u4RefType))
    {
      return TRUE;
    }
    else
    {
      return FALSE;
    }
  }
  else
  {
    if((_ptFBufInfo[u4InstID][u4FBufIdx].ucTFldRefType == u4RefType) || (_ptFBufInfo[u4InstID][u4FBufIdx].ucBFldRefType == u4RefType))
    {
       return TRUE;
    }
    else
    {
      return FALSE;
    }    
  }
}

void vAVC_SLT_InsertRefPicList(UINT32 u4InstID, VDEC_INFO_H264_REF_PIC_LIST_T *ptRefPicList, INT32 iCurrPOC, UINT32 u4RefPicListInfo)
{
  INT32 j;
  UCHAR ucRefType; // 1-> Short 2-> Long
  UCHAR bListType; // 0-> P, 1-> B_0, 2->B_1
  UCHAR ucFBufIdx;
  INT32 iComp0 = 0;
  INT32 iComp1 = 0;
  UINT32 u4Temp;
  BOOL fgSwitch;
  
  ucRefType = u4RefPicListInfo & 0xf;   
  // 0:P_T, 1:P_B, 2:B0_T, 3:B0_B, 4:B1_T, 5:B1_B, 6:P_T_L, 7:P_B_L, 8:B_T_L, 9:B_B_L,  
  bListType = (u4RefPicListInfo >> 8) & 0xf;  
  ucFBufIdx = (u4RefPicListInfo >> 16) & 0xff;  

  if(ucRefType == SREF_PIC)
  {
    // 1st: Insert the current to the last idx
    ptRefPicList->u4FBufIdx[ptRefPicList->u4RefPicCnt] = ucFBufIdx;
    // 2nd: shift Shortterm ref pic        
    for(j=ptRefPicList->u4RefPicCnt - 1; j>=0; j--)
    {
      fgSwitch = FALSE;
      if(bListType == 0)
      {
        if(_ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j+1]].i4TFldPicNum > _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j]].i4TFldPicNum)
        {
          fgSwitch = TRUE;
        }
      }
      else if(bListType == 1)
      {
        if(_ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j+1]].i4BFldPicNum > _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j]].i4BFldPicNum)
        {
          fgSwitch = TRUE;
        }
      }
      else if((bListType == 2) || (bListType == 3))
      {
        iComp0 = (bListType == 2)? _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j]].i4TFldPOC : _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j]].i4BFldPOC;
        iComp1 = (bListType == 2)? _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j+1]].i4TFldPOC : _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j+1]].i4BFldPOC;
        if((fgSLTIsFrmPic(u4InstID)? (((iComp0 < iCurrPOC) &&
                                     (iComp1 < iCurrPOC) &&
                                     (iComp1 > iComp0))
                                     ||
                                     ((iComp0 >= iCurrPOC) &&
                                      (iComp1 < iComp0))
                                     ||
                                     ((iComp0 >= iCurrPOC) &&
                                    (iComp1 < iCurrPOC))) 
                                    :
                                    (((iComp0 <= iCurrPOC) &&
                                     (iComp1 <= iCurrPOC) &&
                                     (iComp1 > iComp0))
                                     ||
                                      ((iComp0 > iCurrPOC) &&
                                      (iComp1 < iComp0))
                                     ||
                                     ((iComp0 > iCurrPOC) &&
                                     (iComp1 <= iCurrPOC)))) )
        {
          fgSwitch = TRUE;
        }
      }
      else if((bListType == 4) || (bListType == 5))
      {
        iComp0 = (bListType == 4)? _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j]].i4TFldPOC : _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j]].i4BFldPOC;
        iComp1 = (bListType == 4)? _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j+1]].i4TFldPOC : _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j+1]].i4BFldPOC;
        if(((iComp0 <= iCurrPOC) &&
             (iComp1 > iComp0))
           ||
           ((iComp0 > iCurrPOC) &&          
             (iComp1 > iCurrPOC) && 
             (iComp1 < iComp0))
           ||
           ((iComp0 <= iCurrPOC) &&
            (iComp1 > iCurrPOC)))            
        {
          fgSwitch = TRUE;
        }
      }
      if(fgSwitch)
      {
        u4Temp = ptRefPicList->u4FBufIdx[j+1];
        ptRefPicList->u4FBufIdx[j+1] = ptRefPicList->u4FBufIdx[j];
        ptRefPicList->u4FBufIdx[j] = u4Temp;
      }
    }
    ptRefPicList->u4RefPicCnt ++;  
  }
  else if(ucRefType == LREF_PIC)
  {
    ptRefPicList->u4FBufIdx[ptRefPicList->u4RefPicCnt] = ucFBufIdx;        
    for(j=(INT32)(ptRefPicList->u4RefPicCnt - 1); j>=0; j--)
    {
      if(bListType == 6)
      {
        iComp0 = _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j]].i4TFldLongTermPicNum;
        iComp1 = _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j+1]].i4TFldLongTermPicNum;
      }
      else if(bListType == 7)
      {
        iComp0 = _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j]].i4BFldLongTermPicNum;
        iComp1 = _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j+1]].i4BFldLongTermPicNum;
      }
      else if(bListType == 8)
      {
        iComp0 = _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j]].u4TFldLongTermFrameIdx;
        iComp1 = _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j+1]].u4TFldLongTermFrameIdx;
      }
      else if(bListType == 9)
      {
        iComp0 = _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j]].u4BFldLongTermFrameIdx;
        iComp1 = _ptFBufInfo[u4InstID][ptRefPicList->u4FBufIdx[j+1]].u4BFldLongTermFrameIdx;
      }            
      if(iComp1 < iComp0)
      {
        u4Temp = ptRefPicList->u4FBufIdx[j+1];
        ptRefPicList->u4FBufIdx[j+1] = ptRefPicList->u4FBufIdx[j];
        ptRefPicList->u4FBufIdx[j] = u4Temp;
      }
    }
    ptRefPicList->u4RefPicCnt ++;
  }
}

void vAVC_SLT_SetupPRefPicList(UINT32 u4InstID, UINT32 *pu4RefIdx, UINT32 u4TFldListIdx, UINT32 u4BFldListIdx)
{
  UINT32 i ;
  UINT32 u4TotalFBuf;  
  UINT32 u4DpbBaseOffset;  
  
  VDEC_INFO_H264_P_REF_PRM_T *prPRefPicListInfo;
  VDEC_HAL_H264_DEC_PRM_T *prH264HalPrm;

  prH264HalPrm = VDec_GetH264HalParam(u4InstID);
  prPRefPicListInfo = &_arPRefPicListInfo[u4InstID];
  u4DpbBaseOffset = 0;  

  u4TotalFBuf = (_ptRefPicList[u4InstID][u4TFldListIdx].u4RefPicCnt >= _ptRefPicList[u4InstID][u4BFldListIdx].u4RefPicCnt)?
                         _ptRefPicList[u4InstID][u4TFldListIdx].u4RefPicCnt : _ptRefPicList[u4InstID][u4BFldListIdx].u4RefPicCnt;
  if(fgSLTIsFrmPic(u4InstID))
  {
    u4TFldListIdx = (_ptRefPicList[u4InstID][u4TFldListIdx].u4RefPicCnt >= _ptRefPicList[u4InstID][u4BFldListIdx].u4RefPicCnt)?
                               u4TFldListIdx : u4BFldListIdx;
  }
  
  for(i=0; i<u4TotalFBuf; i++)
  {
    if(fgSLTIsFrmPic(u4InstID))
    {
      prPRefPicListInfo->ucFBufIdx = _ptRefPicList[u4InstID][u4TFldListIdx].u4FBufIdx[i];
      prPRefPicListInfo->i4BFldLongTermPicNum = _ptFBufInfo[u4InstID][prPRefPicListInfo->ucFBufIdx].i4BFldLongTermPicNum;
      prPRefPicListInfo->i4BFldPicNum = _ptFBufInfo[u4InstID][prPRefPicListInfo->ucFBufIdx].i4BFldPicNum;
      prPRefPicListInfo->i4BFldPOC = _ptFBufInfo[u4InstID][prPRefPicListInfo->ucFBufIdx].i4BFldPOC;
      prPRefPicListInfo->i4PicNum = _ptFBufInfo[u4InstID][prPRefPicListInfo->ucFBufIdx].i4PicNum;
      prPRefPicListInfo->i4LongTermPicNum = _ptFBufInfo[u4InstID][prPRefPicListInfo->ucFBufIdx].i4LongTermPicNum;
      prPRefPicListInfo->i4TFldLongTermPicNum = _ptFBufInfo[u4InstID][prPRefPicListInfo->ucFBufIdx].i4TFldLongTermPicNum;
      prPRefPicListInfo->i4TFldPicNum = _ptFBufInfo[u4InstID][prPRefPicListInfo->ucFBufIdx].i4TFldPicNum;
      prPRefPicListInfo->i4TFldPOC = _ptFBufInfo[u4InstID][prPRefPicListInfo->ucFBufIdx].i4TFldPOC;
      prPRefPicListInfo->u4BFldPara = _ptFBufInfo[u4InstID][prPRefPicListInfo->ucFBufIdx].u4BFldPara;
      prPRefPicListInfo->u4FBufYStartAddr = _ptFBufInfo[u4InstID][prPRefPicListInfo->ucFBufIdx].u4Addr;
      prPRefPicListInfo->u4FBufInfo = FRAME + (i << 8) + (pu4RefIdx[0] << 16);
      prPRefPicListInfo->u4ListIdx = u4TFldListIdx;
      prPRefPicListInfo->u4TFldPara = _ptFBufInfo[u4InstID][prPRefPicListInfo->ucFBufIdx].u4TFldPara;
      prPRefPicListInfo->ucFBufIdx += u4DpbBaseOffset;

      vH264PRefParamConfig(prPRefPicListInfo,&prH264HalPrm->rPRefHalParam);///zhi0308
      vVDEC_HAL_H264_SetPRefPicListReg(u4InstID, &prH264HalPrm->rPRefHalParam);
      
      //vVDecSetPRefPicListReg(FRAME + (i << 8) + (pu4RefIdx[0] << 16), u4TFldListIdx);         
      pu4RefIdx[0] ++;
    }
  }  
}

VOID vAVC_SLT_SetupBRefPicList(UINT32 u4InstID, UINT32 *pu4RefIdx, UINT32 u4TFldListIdx, UINT32 u4BFldListIdx, BOOL *fgDiff)
{
  UINT32 i ;
  UINT32 u4TotalFBuf;
  UINT32 u4Cnt[2];
  BOOL fgIsDiff;
  //VDEC_INFO_H264_B_REF_PRM_T rBRefPicListInfo;
  UINT32 u4DpbBaseOffset;  
   
  VDEC_HAL_H264_DEC_PRM_T *prH264HalPrm;
  VDEC_INFO_H264_B_REF_PRM_T *prBRefPicListInfo;
  prBRefPicListInfo = &_arBRefPicListInfo[u4InstID];
  u4DpbBaseOffset = 0;  

  prH264HalPrm = VDec_GetH264HalParam(u4InstID);
  u4Cnt[0]=pu4RefIdx[0];  
  u4Cnt[1]=pu4RefIdx[0]; 

  u4TotalFBuf = (_ptRefPicList[u4InstID][u4TFldListIdx].u4RefPicCnt >= _ptRefPicList[u4InstID][u4BFldListIdx].u4RefPicCnt)?
                         _ptRefPicList[u4InstID][u4TFldListIdx].u4RefPicCnt : _ptRefPicList[u4InstID][u4BFldListIdx].u4RefPicCnt;
  if(fgSLTIsFrmPic(u4InstID))
  {
    u4TFldListIdx = (_ptRefPicList[u4InstID][u4TFldListIdx].u4RefPicCnt >= _ptRefPicList[u4InstID][u4BFldListIdx].u4RefPicCnt)? u4TFldListIdx : u4BFldListIdx;
  }  

  for(i=0; i<u4TotalFBuf; i++)
  {
    if(fgSLTIsFrmPic(u4InstID))
    {
      prBRefPicListInfo->ucFBufIdx = _ptRefPicList[u4InstID][u4TFldListIdx].u4FBufIdx[i];
      prBRefPicListInfo->i4BFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldLongTermPicNum;
      prBRefPicListInfo->i4BFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPicNum;
      prBRefPicListInfo->i4BFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPOC;
      prBRefPicListInfo->i4PicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4PicNum;    
      prBRefPicListInfo->i4LongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4LongTermPicNum;
      prBRefPicListInfo->i4TFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldLongTermPicNum;
      prBRefPicListInfo->i4TFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPicNum;
      prBRefPicListInfo->i4TFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPOC;
      //prBRefPicListInfo->u4DramPicSize = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize;
      prBRefPicListInfo->u4BFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4BFldPara;
      prBRefPicListInfo->u4FBufYStartAddr = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4Addr;
      prBRefPicListInfo->u4FBufMvStartAddr = (prBRefPicListInfo->u4FBufYStartAddr  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize * 3) >>1));
      prBRefPicListInfo->u4FBufInfo = FRAME + (i << 8) + (pu4RefIdx[0] << 16);
      prBRefPicListInfo->u4ListIdx = u4TFldListIdx;
      prBRefPicListInfo->u4TFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4TFldPara;
      if(prBRefPicListInfo->u4ListIdx < 4)
      {
        prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx + 2;
      }
      else
      {
        prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx;
      }
      prBRefPicListInfo->ucFBufIdx1 = _ptRefPicList[u4InstID][prBRefPicListInfo->u4ListIdx1].u4FBufIdx[i];
      prBRefPicListInfo->u4FBufYStartAddr1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4Addr;;
      prBRefPicListInfo->i4LongTermPicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4LongTermPicNum;
      prBRefPicListInfo->i4PicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4PicNum;;
      prBRefPicListInfo->i4TFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4TFldPOC;
      prBRefPicListInfo->i4BFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPOC;
      prBRefPicListInfo->u4TFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4TFldPara;
      prBRefPicListInfo->u4BFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4BFldPara;
      prBRefPicListInfo->u4FBufMvStartAddr1 = (prBRefPicListInfo->u4FBufYStartAddr1  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4DramPicSize * 3) >>1));
      prBRefPicListInfo->ucFBufIdx += u4DpbBaseOffset;
      prBRefPicListInfo->ucFBufIdx1 += u4DpbBaseOffset;
      vH264BRefParamConfig(prBRefPicListInfo,&prH264HalPrm->rBRefHalParam);
      fgIsDiff = bVDEC_HAL_H264_SetBRefPicListReg(u4InstID, &prH264HalPrm->rBRefHalParam);
      //vVDecSetBRefPicListReg(FRAME + (i << 8) + (pu4RefIdx[0] << 16), u4TFldListIdx);

      if((!fgDiff[0]) && fgIsDiff)
      {
        fgDiff[0] = TRUE;
      }
      pu4RefIdx[0] ++;
    }
    else if(_tVDecPrm[u4InstID].ucPicStruct == TOP_FIELD)
    {
      if(i < _ptRefPicList[u4InstID][u4TFldListIdx].u4RefPicCnt)
      {
        prBRefPicListInfo->ucFBufIdx = _ptRefPicList[u4InstID][u4TFldListIdx].u4FBufIdx[i];
        prBRefPicListInfo->i4BFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldLongTermPicNum;
        prBRefPicListInfo->i4BFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPicNum;
        prBRefPicListInfo->i4BFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPOC;
        prBRefPicListInfo->i4PicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4PicNum;    
        prBRefPicListInfo->i4LongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4LongTermPicNum;
        prBRefPicListInfo->i4TFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldLongTermPicNum;
        prBRefPicListInfo->i4TFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPicNum;
        prBRefPicListInfo->i4TFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPOC;
        //prBRefPicListInfo->u4DramPicSize = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize;
        prBRefPicListInfo->u4BFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4BFldPara;
        prBRefPicListInfo->u4FBufYStartAddr = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4Addr;
        prBRefPicListInfo->u4FBufMvStartAddr = (prBRefPicListInfo->u4FBufYStartAddr  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize * 3) >>1));
        prBRefPicListInfo->u4FBufInfo = TOP_FIELD + (i << 8) + (pu4RefIdx[0] << 16);
        prBRefPicListInfo->u4ListIdx = u4TFldListIdx;
        prBRefPicListInfo->u4TFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4TFldPara;
        if(prBRefPicListInfo->u4ListIdx < 4)
        {
          prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx + 2;
        }
        else
        {
          prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx;
        }
        prBRefPicListInfo->ucFBufIdx1 = _ptRefPicList[u4InstID][prBRefPicListInfo->u4ListIdx1].u4FBufIdx[i];
        prBRefPicListInfo->u4FBufYStartAddr1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4Addr;;
        prBRefPicListInfo->i4LongTermPicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4LongTermPicNum;
        prBRefPicListInfo->i4PicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPicNum;;
        prBRefPicListInfo->i4TFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4TFldPOC;
        prBRefPicListInfo->i4BFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPOC;
        prBRefPicListInfo->u4TFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4TFldPara;
        prBRefPicListInfo->u4BFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4BFldPara;
        prBRefPicListInfo->u4FBufMvStartAddr1 = (prBRefPicListInfo->u4FBufYStartAddr1  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4DramPicSize * 3) >>1));
        prBRefPicListInfo->ucFBufIdx += u4DpbBaseOffset;
        prBRefPicListInfo->ucFBufIdx1 += u4DpbBaseOffset;
        vH264BRefParamConfig(prBRefPicListInfo,&prH264HalPrm->rBRefHalParam);
        fgIsDiff = bVDEC_HAL_H264_SetBRefPicListReg(u4InstID, &prH264HalPrm->rBRefHalParam);
        if(u4TFldListIdx < 4) // Short-term only
        {
          prBRefPicListInfo->ucFBufIdx = _ptRefPicList[u4InstID][u4TFldListIdx+2].u4FBufIdx[i];
          prBRefPicListInfo->i4BFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldLongTermPicNum;
          prBRefPicListInfo->i4BFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPicNum;
          prBRefPicListInfo->i4BFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPOC;
          prBRefPicListInfo->i4PicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4PicNum;    
          prBRefPicListInfo->i4LongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4LongTermPicNum;
          prBRefPicListInfo->i4TFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldLongTermPicNum;
          prBRefPicListInfo->i4TFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPicNum;
          prBRefPicListInfo->i4TFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPOC;
          //prBRefPicListInfo->u4DramPicSize = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize;
          prBRefPicListInfo->u4BFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4BFldPara;
          prBRefPicListInfo->u4FBufYStartAddr = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4Addr;
          prBRefPicListInfo->u4FBufMvStartAddr = (prBRefPicListInfo->u4FBufYStartAddr  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize * 3) >>1));
          prBRefPicListInfo->u4FBufInfo = TOP_FIELD + (i << 8) + (pu4RefIdx[0] << 16);
          prBRefPicListInfo->u4ListIdx = u4TFldListIdx+2;
          prBRefPicListInfo->u4TFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4TFldPara;
          if(prBRefPicListInfo->u4ListIdx < 4)
          {
            prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx + 2;
          }
          else
          {
            prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx;
          }
          prBRefPicListInfo->ucFBufIdx1 = _ptRefPicList[u4InstID][prBRefPicListInfo->u4ListIdx1].u4FBufIdx[i];
          prBRefPicListInfo->u4FBufYStartAddr1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4Addr;;
          prBRefPicListInfo->i4LongTermPicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4LongTermPicNum;
          prBRefPicListInfo->i4PicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPicNum;;
          prBRefPicListInfo->i4TFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4TFldPOC;
          prBRefPicListInfo->i4BFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPOC;
          prBRefPicListInfo->u4TFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4TFldPara;
          prBRefPicListInfo->u4BFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4BFldPara;
          prBRefPicListInfo->u4FBufMvStartAddr1 = (prBRefPicListInfo->u4FBufYStartAddr1  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4DramPicSize * 3) >>1));
          prBRefPicListInfo->ucFBufIdx += u4DpbBaseOffset;
          prBRefPicListInfo->ucFBufIdx1 += u4DpbBaseOffset;
          vH264BRefParamConfig(prBRefPicListInfo,&prH264HalPrm->rBRefHalParam);
          fgIsDiff = bVDEC_HAL_H264_SetBRefPicListReg(u4InstID, &prH264HalPrm->rBRefHalParam);
        }

        if((!fgDiff[0]) && fgIsDiff)
        {
          fgDiff[0] = TRUE;
        }
        pu4RefIdx[0] ++;
        u4Cnt[0] ++;
      }
      if(i < _ptRefPicList[u4InstID][u4BFldListIdx].u4RefPicCnt)
      {
        prBRefPicListInfo->ucFBufIdx = _ptRefPicList[u4InstID][u4BFldListIdx].u4FBufIdx[i];
        prBRefPicListInfo->i4BFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldLongTermPicNum;
        prBRefPicListInfo->i4BFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPicNum;
        prBRefPicListInfo->i4BFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPOC;
        prBRefPicListInfo->i4PicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4PicNum;    
        prBRefPicListInfo->i4LongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4LongTermPicNum;
        prBRefPicListInfo->i4TFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldLongTermPicNum;
        prBRefPicListInfo->i4TFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPicNum;
        prBRefPicListInfo->i4TFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPOC;
        prBRefPicListInfo->u4BFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4BFldPara;
        prBRefPicListInfo->u4FBufYStartAddr = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4Addr;
        prBRefPicListInfo->u4FBufMvStartAddr = (prBRefPicListInfo->u4FBufYStartAddr  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize * 3) >>1));
        prBRefPicListInfo->u4FBufInfo = BOTTOM_FIELD + (i << 8) + (pu4RefIdx[0] << 16);
        prBRefPicListInfo->u4ListIdx = u4BFldListIdx;
        prBRefPicListInfo->u4TFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4TFldPara;
        if(prBRefPicListInfo->u4ListIdx < 4)
        {
          prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx + 2;
        }
        else
        {
          prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx;
        }
        prBRefPicListInfo->ucFBufIdx1 = _ptRefPicList[u4InstID][prBRefPicListInfo->u4ListIdx1].u4FBufIdx[i];
        prBRefPicListInfo->u4FBufYStartAddr1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4Addr;;
        prBRefPicListInfo->i4LongTermPicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4LongTermPicNum;
        prBRefPicListInfo->i4PicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPicNum;;
        prBRefPicListInfo->i4TFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4TFldPOC;
        prBRefPicListInfo->i4BFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPOC;
        prBRefPicListInfo->u4TFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4TFldPara;
        prBRefPicListInfo->u4BFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4BFldPara;
        prBRefPicListInfo->u4FBufMvStartAddr1 = (prBRefPicListInfo->u4FBufYStartAddr1  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4DramPicSize * 3) >>1));
        prBRefPicListInfo->ucFBufIdx += u4DpbBaseOffset;
        prBRefPicListInfo->ucFBufIdx1 += u4DpbBaseOffset;        
        vH264BRefParamConfig(prBRefPicListInfo,&prH264HalPrm->rBRefHalParam);
        fgIsDiff = bVDEC_HAL_H264_SetBRefPicListReg(u4InstID, &prH264HalPrm->rBRefHalParam);
        if(u4BFldListIdx < 4) // Short-term only
        {
          prBRefPicListInfo->ucFBufIdx = _ptRefPicList[u4InstID][u4BFldListIdx+2].u4FBufIdx[i];
          prBRefPicListInfo->i4BFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldLongTermPicNum;
          prBRefPicListInfo->i4BFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPicNum;
          prBRefPicListInfo->i4BFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPOC;
          prBRefPicListInfo->i4PicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4PicNum;    
          prBRefPicListInfo->i4LongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4LongTermPicNum;
          prBRefPicListInfo->i4TFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldLongTermPicNum;
          prBRefPicListInfo->i4TFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPicNum;
          prBRefPicListInfo->i4TFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPOC;
          //prBRefPicListInfo->u4DramPicSize = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize;
          prBRefPicListInfo->u4BFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4BFldPara;
          prBRefPicListInfo->u4FBufYStartAddr = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4Addr;
          prBRefPicListInfo->u4FBufMvStartAddr = (prBRefPicListInfo->u4FBufYStartAddr  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize * 3) >>1));
          prBRefPicListInfo->u4FBufInfo = BOTTOM_FIELD + (i << 8) + (pu4RefIdx[0] << 16);
          prBRefPicListInfo->u4ListIdx = u4BFldListIdx+2;
          prBRefPicListInfo->u4TFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4TFldPara;
          if(prBRefPicListInfo->u4ListIdx < 4)
          {
            prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx + 2;
          }
          else
          {
            prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx;
          }
          prBRefPicListInfo->ucFBufIdx1 = _ptRefPicList[u4InstID][prBRefPicListInfo->u4ListIdx1].u4FBufIdx[i];
          prBRefPicListInfo->u4FBufYStartAddr1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4Addr;;
          prBRefPicListInfo->i4LongTermPicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4LongTermPicNum;
          prBRefPicListInfo->i4PicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPicNum;;
          prBRefPicListInfo->i4TFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4TFldPOC;
          prBRefPicListInfo->i4BFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPOC;
          prBRefPicListInfo->u4TFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4TFldPara;
          prBRefPicListInfo->u4BFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4BFldPara;
          prBRefPicListInfo->u4FBufMvStartAddr1 = (prBRefPicListInfo->u4FBufYStartAddr1  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4DramPicSize * 3) >>1));
          prBRefPicListInfo->ucFBufIdx += u4DpbBaseOffset;
          prBRefPicListInfo->ucFBufIdx1 += u4DpbBaseOffset;
          vH264BRefParamConfig(prBRefPicListInfo,&prH264HalPrm->rBRefHalParam);
          fgIsDiff = bVDEC_HAL_H264_SetBRefPicListReg(u4InstID, &prH264HalPrm->rBRefHalParam);
        }

        if((!fgDiff[0]) && fgIsDiff)
        {
          fgDiff[0] = TRUE;
        }
        pu4RefIdx[0] ++;
        u4Cnt[1] ++;        
      }
    }
    else if(_tVDecPrm[u4InstID].ucPicStruct == BOTTOM_FIELD)
    {
      if(i < _ptRefPicList[u4InstID][u4BFldListIdx].u4RefPicCnt)
      {
        prBRefPicListInfo->ucFBufIdx = _ptRefPicList[u4InstID][u4BFldListIdx].u4FBufIdx[i];
        prBRefPicListInfo->i4BFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldLongTermPicNum;
        prBRefPicListInfo->i4BFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPicNum;
        prBRefPicListInfo->i4BFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPOC;
        prBRefPicListInfo->i4PicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4PicNum;    
        prBRefPicListInfo->i4LongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4LongTermPicNum;
        prBRefPicListInfo->i4TFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldLongTermPicNum;
        prBRefPicListInfo->i4TFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPicNum;
        prBRefPicListInfo->i4TFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPOC;
        //prBRefPicListInfo->u4DramPicSize = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize;
        prBRefPicListInfo->u4BFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4BFldPara;
        prBRefPicListInfo->u4FBufYStartAddr = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4Addr;
        prBRefPicListInfo->u4FBufMvStartAddr = (prBRefPicListInfo->u4FBufYStartAddr  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize * 3) >>1));
        prBRefPicListInfo->u4FBufInfo = BOTTOM_FIELD + (i << 8) + (pu4RefIdx[0] << 16);
        prBRefPicListInfo->u4ListIdx = u4BFldListIdx;
        prBRefPicListInfo->u4TFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4TFldPara;
        if(prBRefPicListInfo->u4ListIdx < 4)
        {
          prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx + 2;
        }
        else
        {
          prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx;
        }
        prBRefPicListInfo->ucFBufIdx1 = _ptRefPicList[u4InstID][prBRefPicListInfo->u4ListIdx1].u4FBufIdx[i];
        prBRefPicListInfo->u4FBufYStartAddr1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4Addr;;
        prBRefPicListInfo->i4LongTermPicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4LongTermPicNum;
        prBRefPicListInfo->i4PicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPicNum;;
        prBRefPicListInfo->i4TFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4TFldPOC;
        prBRefPicListInfo->i4BFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPOC;
        prBRefPicListInfo->u4TFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4TFldPara;
        prBRefPicListInfo->u4BFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4BFldPara;
        prBRefPicListInfo->u4FBufMvStartAddr1 = (prBRefPicListInfo->u4FBufYStartAddr1  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4DramPicSize * 3) >>1));
        prBRefPicListInfo->ucFBufIdx += u4DpbBaseOffset;
        prBRefPicListInfo->ucFBufIdx1 += u4DpbBaseOffset;        
        vH264BRefParamConfig(prBRefPicListInfo,&prH264HalPrm->rBRefHalParam);
        fgIsDiff = bVDEC_HAL_H264_SetBRefPicListReg(u4InstID, &prH264HalPrm->rBRefHalParam);
        //vVDecSetBRefPicListReg(BOTTOM_FIELD + (i << 8) + (pu4RefIdx[0] << 16), u4BFldListIdx);
        if(u4BFldListIdx < 4) // Short-term only
        {
          prBRefPicListInfo->ucFBufIdx = _ptRefPicList[u4InstID][u4BFldListIdx+2].u4FBufIdx[i];
          prBRefPicListInfo->i4BFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldLongTermPicNum;
          prBRefPicListInfo->i4BFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPicNum;
          prBRefPicListInfo->i4BFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPOC;
          prBRefPicListInfo->i4PicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4PicNum;    
          prBRefPicListInfo->i4LongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4LongTermPicNum;
          prBRefPicListInfo->i4TFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldLongTermPicNum;
          prBRefPicListInfo->i4TFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPicNum;
          prBRefPicListInfo->i4TFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPOC;
          //prBRefPicListInfo->u4DramPicSize = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize;
          prBRefPicListInfo->u4BFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4BFldPara;
          prBRefPicListInfo->u4FBufYStartAddr = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4Addr;
          prBRefPicListInfo->u4FBufMvStartAddr = (prBRefPicListInfo->u4FBufYStartAddr  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize * 3) >>1));
          prBRefPicListInfo->u4FBufInfo = BOTTOM_FIELD + (i << 8) + (pu4RefIdx[0] << 16);
          prBRefPicListInfo->u4ListIdx = u4BFldListIdx+2;
          prBRefPicListInfo->u4TFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4TFldPara;
          if(prBRefPicListInfo->u4ListIdx < 4)
          {
            prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx + 2;
          }
          else
          {
            prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx;
          }
          prBRefPicListInfo->ucFBufIdx1 = _ptRefPicList[u4InstID][prBRefPicListInfo->u4ListIdx1].u4FBufIdx[i];
          prBRefPicListInfo->u4FBufYStartAddr1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4Addr;;
          prBRefPicListInfo->i4LongTermPicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4LongTermPicNum;
          prBRefPicListInfo->i4PicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPicNum;;
          prBRefPicListInfo->i4TFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4TFldPOC;
          prBRefPicListInfo->i4BFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPOC;
          prBRefPicListInfo->u4TFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4TFldPara;
          prBRefPicListInfo->u4BFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4BFldPara;
          prBRefPicListInfo->u4FBufMvStartAddr1 = (prBRefPicListInfo->u4FBufYStartAddr1  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4DramPicSize * 3) >>1));
          prBRefPicListInfo->ucFBufIdx += u4DpbBaseOffset;
          prBRefPicListInfo->ucFBufIdx1 += u4DpbBaseOffset;
          vH264BRefParamConfig(prBRefPicListInfo,&prH264HalPrm->rBRefHalParam);
          fgIsDiff = bVDEC_HAL_H264_SetBRefPicListReg(u4InstID, &prH264HalPrm->rBRefHalParam);
          //vVDecSetBRefPicListReg(BOTTOM_FIELD + (i << 8) + (pu4RefIdx[0] << 16), u4BFldListIdx+2);
        }

        if((!fgDiff[0]) && fgIsDiff)
        {
          fgDiff[0] = TRUE;
        }
        pu4RefIdx[0] ++;
        u4Cnt[1] ++;        
      }      
      if(i < _ptRefPicList[u4InstID][u4TFldListIdx].u4RefPicCnt)
      {
        prBRefPicListInfo->ucFBufIdx = _ptRefPicList[u4InstID][u4TFldListIdx].u4FBufIdx[i];
        prBRefPicListInfo->i4BFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldLongTermPicNum;
        prBRefPicListInfo->i4BFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPicNum;
        prBRefPicListInfo->i4BFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPOC;
        prBRefPicListInfo->i4PicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4PicNum;    
        prBRefPicListInfo->i4LongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4LongTermPicNum;
        prBRefPicListInfo->i4TFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldLongTermPicNum;
        prBRefPicListInfo->i4TFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPicNum;
        prBRefPicListInfo->i4TFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPOC;        
        //prBRefPicListInfo->u4DramPicSize = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize;
        prBRefPicListInfo->u4BFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4BFldPara;
        prBRefPicListInfo->u4FBufYStartAddr = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4Addr;
        prBRefPicListInfo->u4FBufMvStartAddr = (prBRefPicListInfo->u4FBufYStartAddr  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize * 3) >>1));
        prBRefPicListInfo->u4FBufInfo = TOP_FIELD + (i << 8) + (pu4RefIdx[0] << 16);
        prBRefPicListInfo->u4ListIdx = u4TFldListIdx;
        prBRefPicListInfo->u4TFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4TFldPara;
        if(prBRefPicListInfo->u4ListIdx < 4)
        {
          prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx + 2;
        }
        else
        {
          prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx;
        }
        prBRefPicListInfo->ucFBufIdx1 = _ptRefPicList[u4InstID][prBRefPicListInfo->u4ListIdx1].u4FBufIdx[i];
        prBRefPicListInfo->u4FBufYStartAddr1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4Addr;;
        prBRefPicListInfo->i4LongTermPicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4LongTermPicNum;
        prBRefPicListInfo->i4PicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPicNum;;
        prBRefPicListInfo->i4TFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4TFldPOC;
        prBRefPicListInfo->i4BFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPOC;
        prBRefPicListInfo->u4TFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4TFldPara;
        prBRefPicListInfo->u4BFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4BFldPara;
        prBRefPicListInfo->u4FBufMvStartAddr1 = (prBRefPicListInfo->u4FBufYStartAddr1  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4DramPicSize * 3) >>1));
        prBRefPicListInfo->ucFBufIdx += u4DpbBaseOffset;
        prBRefPicListInfo->ucFBufIdx1 += u4DpbBaseOffset;
        vH264BRefParamConfig(prBRefPicListInfo,&prH264HalPrm->rBRefHalParam);
        fgIsDiff = bVDEC_HAL_H264_SetBRefPicListReg(u4InstID, &prH264HalPrm->rBRefHalParam);
        //vVDecSetBRefPicListReg(TOP_FIELD + (i << 8) + (pu4RefIdx[0] << 16), u4TFldListIdx);
        if(u4TFldListIdx < 4) // Short-term only
        {
          prBRefPicListInfo->ucFBufIdx = _ptRefPicList[u4InstID][u4TFldListIdx+2].u4FBufIdx[i];
          prBRefPicListInfo->i4BFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldLongTermPicNum;
          prBRefPicListInfo->i4BFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPicNum;
          prBRefPicListInfo->i4BFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4BFldPOC;
          prBRefPicListInfo->i4PicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4PicNum;    
          prBRefPicListInfo->i4LongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4LongTermPicNum;
          prBRefPicListInfo->i4TFldLongTermPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldLongTermPicNum;
          prBRefPicListInfo->i4TFldPicNum = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPicNum;
          prBRefPicListInfo->i4TFldPOC = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].i4TFldPOC;
          //prBRefPicListInfo->u4DramPicSize = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize;
          prBRefPicListInfo->u4BFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4BFldPara;
          prBRefPicListInfo->u4FBufYStartAddr = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4Addr;
          prBRefPicListInfo->u4FBufMvStartAddr = (prBRefPicListInfo->u4FBufYStartAddr  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4DramPicSize * 3) >>1));
          prBRefPicListInfo->u4FBufInfo = TOP_FIELD + (i << 8) + (pu4RefIdx[0] << 16);
          prBRefPicListInfo->u4ListIdx = u4TFldListIdx+2;
          prBRefPicListInfo->u4TFldPara = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx].u4TFldPara;
          if(prBRefPicListInfo->u4ListIdx < 4)
          {
            prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx + 2;
          }
          else
          {
            prBRefPicListInfo->u4ListIdx1 = prBRefPicListInfo->u4ListIdx;
          }
          prBRefPicListInfo->ucFBufIdx1 = _ptRefPicList[u4InstID][prBRefPicListInfo->u4ListIdx1].u4FBufIdx[i];
          prBRefPicListInfo->u4FBufYStartAddr1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4Addr;;
          prBRefPicListInfo->i4LongTermPicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4LongTermPicNum;
          prBRefPicListInfo->i4PicNum1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPicNum;;
          prBRefPicListInfo->i4TFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4TFldPOC;
          prBRefPicListInfo->i4BFldPOC1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].i4BFldPOC;
          prBRefPicListInfo->u4TFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4TFldPara;
          prBRefPicListInfo->u4BFldPara1 = _ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4BFldPara;
          prBRefPicListInfo->u4FBufMvStartAddr1 = (prBRefPicListInfo->u4FBufYStartAddr1  + ((_ptFBufInfo[u4InstID][prBRefPicListInfo->ucFBufIdx1].u4DramPicSize * 3) >>1));
          prBRefPicListInfo->ucFBufIdx += u4DpbBaseOffset;
          prBRefPicListInfo->ucFBufIdx1 += u4DpbBaseOffset;
          vH264BRefParamConfig(prBRefPicListInfo,&prH264HalPrm->rBRefHalParam);
          fgIsDiff = bVDEC_HAL_H264_SetBRefPicListReg(u4InstID, &prH264HalPrm->rBRefHalParam);
        }
        if((!fgDiff[0]) && fgIsDiff)
        {
          fgDiff[0] = TRUE;
        }
        pu4RefIdx[0] ++;
        u4Cnt[0] ++;        
      }
    }

  } 
  
}

VOID vAVC_SLT_VDecSetPRefPicList(UINT32 u4InstID)
{
  INT32 i;
  UINT32 u4AddTop;
  UINT32 u4AddBot;
  UINT32 u4Temp;
  UINT32 u4CurrPicNum;
  UINT32 u4Idx;

  _ptCurrFBufInfo[u4InstID]->u4FrameNum = _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4FrameNum;
  u4CurrPicNum = (fgSLTIsFrmPic(u4InstID))? _ptCurrFBufInfo[u4InstID]->u4FrameNum : ((_ptCurrFBufInfo[u4InstID]->u4FrameNum << 1) +1);
  _ptCurrFBufInfo[u4InstID]->i4PicNum = u4CurrPicNum;
  _ptRefPicList[u4InstID][0].u4RefPicCnt = 0;
  _ptRefPicList[u4InstID][1].u4RefPicCnt = 0;  
  _ptRefPicList[u4InstID][4].u4RefPicCnt = 0;
  _ptRefPicList[u4InstID][5].u4RefPicCnt = 0;

  //if(fgIsFrmPic(_u4VDecID))
  {
    for(i=0; i<_tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum; i++)
    {
      if(_tVDecPrm[u4InstID].ucPicStruct == TOP_FIELD)
      {
        u4AddTop = 1;
        u4AddBot = 0;
      }
      else if(_tVDecPrm[u4InstID].ucPicStruct == BOTTOM_FIELD)
      {
        u4AddTop = 0;
        u4AddBot = 1;
      }
      else
      {
        u4AddTop = 0;
        u4AddBot = 0;
      }
      
      if(fgAVC_SLT_ChkRefInfo(u4InstID, i, SREF_PIC))
      {        
        if(_ptFBufInfo[u4InstID][i].u4FrameNum > _ptCurrFBufInfo[u4InstID]->u4FrameNum)
        {
          _ptFBufInfo[u4InstID][i].i4FrameNumWrap = _ptFBufInfo[u4InstID][i].u4FrameNum - _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prSPS->u4MaxFrameNum;
        }
        else
        {
          _ptFBufInfo[u4InstID][i].i4FrameNumWrap = _ptFBufInfo[u4InstID][i].u4FrameNum;
        }
        if(fgSLTIsFrmPic(u4InstID))
        {
          _ptFBufInfo[u4InstID][i].i4PicNum = _ptFBufInfo[u4InstID][i].i4FrameNumWrap;
        }
        else
        {
          _ptFBufInfo[u4InstID][i].i4PicNum = (_ptFBufInfo[u4InstID][i].i4FrameNumWrap<<1) + 1;
        }
        
        if(_ptFBufInfo[u4InstID][i].ucTFldRefType == SREF_PIC)
        {
          _ptFBufInfo[u4InstID][i].i4TFldPicNum = (_ptFBufInfo[u4InstID][i].i4FrameNumWrap << 1) + u4AddTop;
          vAVC_SLT_InsertRefPicList(u4InstID, &_ptRefPicList[u4InstID][0], i, SREF_PIC + (0 <<8) + ( i<<16));          
        }
        if(_ptFBufInfo[u4InstID][i].ucBFldRefType == SREF_PIC)
        {
          _ptFBufInfo[u4InstID][i].i4BFldPicNum = (_ptFBufInfo[u4InstID][i].i4FrameNumWrap << 1) + u4AddBot;
          vAVC_SLT_InsertRefPicList(u4InstID, &_ptRefPicList[u4InstID][1], i, SREF_PIC + (1 <<8) + ( i<<16));          
        }
      }
      else if(fgAVC_SLT_ChkRefInfo(u4InstID, i, LREF_PIC))
      {        
        _ptFBufInfo[u4InstID][i].i4LongTermPicNum = _ptFBufInfo[u4InstID][i].u4LongTermFrameIdx;
        
        if(_ptFBufInfo[u4InstID][i].ucTFldRefType == LREF_PIC)
        {
          u4Idx = _ptFBufInfo[u4InstID][i].u4TFldLongTermFrameIdx;
          _ptFBufInfo[u4InstID][i].i4TFldLongTermPicNum = (u4Idx << 1) + u4AddTop;
          vAVC_SLT_InsertRefPicList(u4InstID, &_ptRefPicList[u4InstID][4], i, LREF_PIC + (6 <<8) + ( i<<16));          
        } 
        if( _ptFBufInfo[u4InstID][i].ucBFldRefType == LREF_PIC)
        {
          _ptFBufInfo[u4InstID][i].i4BFldLongTermPicNum = (_ptFBufInfo[u4InstID][i].u4BFldLongTermFrameIdx << 1) + u4AddBot;        
          vAVC_SLT_InsertRefPicList(u4InstID, &_ptRefPicList[u4InstID][5], i, LREF_PIC + (7 <<8) + ( i<<16));          
        }        
      }      
    }
  }

  vVDEC_HAL_H264_InitPRefList(u4InstID, fgSLTIsFrmPic(u4InstID), _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prSPS->u4MaxFrameNum, u4CurrPicNum);
  //fprintf(_tRecFileInfo.fpFile, "frame num = %d\n", _ptCurrFBufInfo->u4FrameNum);    
  u4Temp = 0;
  vAVC_SLT_SetupPRefPicList(u4InstID, &u4Temp, 0, 1);
  vAVC_SLT_SetupPRefPicList(u4InstID, &u4Temp, 4, 5);
}


VOID vAVC_SLT_VDecSetBRefPicList(UINT32 u4InstID)
{
  INT32 i;
  UINT32 u4Temp;
  INT32 iCurrPOC;
  UINT32 u4TotalRPIdx;
  BOOL fgDiff;
  VDEC_INFO_H264_POC_PRM_T rPOCInfo;
  
  vAVC_SLT_VDecSetCurrPOC(u4InstID);   
  if(fgSLTIsFrmPic(u4InstID))
  {
    iCurrPOC = _ptCurrFBufInfo[u4InstID]->i4POC;
  }
  else
  {
    iCurrPOC = (_tVDecPrm[u4InstID].ucPicStruct == TOP_FIELD)? _ptCurrFBufInfo[u4InstID]->i4TFldPOC : _ptCurrFBufInfo[u4InstID]->i4BFldPOC;
  }
  rPOCInfo.ucPicStruct = _tVDecPrm[u4InstID].ucPicStruct;
  rPOCInfo.fgIsFrmPic = fgSLTIsFrmPic(u4InstID);
  rPOCInfo.i4BFldPOC = _ptCurrFBufInfo[u4InstID]->i4BFldPOC;
  rPOCInfo.i4POC = _ptCurrFBufInfo[u4InstID]->i4POC;
  rPOCInfo.i4TFldPOC = _ptCurrFBufInfo[u4InstID]->i4TFldPOC;
  vVDEC_HAL_H264_SetPOC( u4InstID, &rPOCInfo);
  

  _ptRefPicList[u4InstID][0].u4RefPicCnt = 0;  
  _ptRefPicList[u4InstID][1].u4RefPicCnt = 0;  
  _ptRefPicList[u4InstID][2].u4RefPicCnt = 0;  
  _ptRefPicList[u4InstID][3].u4RefPicCnt = 0;  
  _ptRefPicList[u4InstID][4].u4RefPicCnt = 0;  
  _ptRefPicList[u4InstID][5].u4RefPicCnt = 0;  

  //if(fgIsFrmPic(_u4VDecID))
  {
    for(i=0; i<_tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum; i++)
    {     
      if(fgAVC_SLT_ChkRefInfo(u4InstID, i, SREF_PIC))
      {
        // Avoid non-existing pic into ref pic list when POC type = 0
        if((_ptFBufInfo[u4InstID][i].ucTFldRefType == SREF_PIC)) 
        {
          // B0
          vAVC_SLT_InsertRefPicList(u4InstID, &_ptRefPicList[u4InstID][0], iCurrPOC, SREF_PIC + (2 <<8) + (i<<16));
          // B1
          vAVC_SLT_InsertRefPicList(u4InstID, &_ptRefPicList[u4InstID][2], iCurrPOC, SREF_PIC + (4 <<8) + (i<<16));      
        }
        if((_ptFBufInfo[u4InstID][i].ucBFldRefType == SREF_PIC) 
            && !((_tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prSPS->u4PicOrderCntType == 0) && (_ptFBufInfo[u4InstID][i].fgNonExisting)))
        {
          // B0
          vAVC_SLT_InsertRefPicList(u4InstID, &_ptRefPicList[u4InstID][1], iCurrPOC, SREF_PIC + (3 <<8) + (i <<16));
          // B1
          vAVC_SLT_InsertRefPicList(u4InstID, &_ptRefPicList[u4InstID][3], iCurrPOC, SREF_PIC + (5 <<8) + (i <<16));    
        }
      }
      else if(fgAVC_SLT_ChkRefInfo(u4InstID, i, LREF_PIC))
      {
        if(_ptFBufInfo[u4InstID][i].ucTFldRefType == LREF_PIC)
        {
          vAVC_SLT_InsertRefPicList(u4InstID, &_ptRefPicList[u4InstID][4], iCurrPOC, LREF_PIC + (8 <<8) + (i <<16));
        }
        if(_ptFBufInfo[u4InstID][i].ucBFldRefType == LREF_PIC)
        {
          vAVC_SLT_InsertRefPicList(u4InstID, &_ptRefPicList[u4InstID][5], iCurrPOC, LREF_PIC + (9 <<8) + (i <<16)); 
        }        
      }      
    }
  }

  vVDEC_HAL_H264_InitBRefList(u4InstID);
  
  //vWriteAVCVLD(RW_AVLD_RESET_PIC_NUM, RESET_PIC_NUM);
  u4Temp = 0;
  fgDiff = FALSE;
  //fprintf(_tRecFileInfo.fpFile,"B0 \n");  
  vAVC_SLT_SetupBRefPicList(u4InstID, &u4Temp, 0, 1, &fgDiff);
  vAVC_SLT_SetupBRefPicList(u4InstID, &u4Temp, 4, 5, &fgDiff);

  if(fgSLTIsFrmPic(u4InstID))
  {
    u4TotalRPIdx = (_ptRefPicList[u4InstID][0].u4RefPicCnt < _ptRefPicList[u4InstID][1].u4RefPicCnt)? _ptRefPicList[u4InstID][1].u4RefPicCnt : _ptRefPicList[u4InstID][0].u4RefPicCnt;
    u4TotalRPIdx += (_ptRefPicList[u4InstID][4].u4RefPicCnt < _ptRefPicList[u4InstID][5].u4RefPicCnt)? _ptRefPicList[u4InstID][5].u4RefPicCnt : _ptRefPicList[u4InstID][4].u4RefPicCnt;
  }
  else
  {
    u4TotalRPIdx = _ptRefPicList[u4InstID][0].u4RefPicCnt + _ptRefPicList[u4InstID][1].u4RefPicCnt + _ptRefPicList[u4InstID][4].u4RefPicCnt + _ptRefPicList[u4InstID][5].u4RefPicCnt;
  }

  
  // in field pic, if B0 & B1 identical, switch the 1st 2 items
  if(u4TotalRPIdx > 1 && (!fgDiff))
  {
    vVDEC_HAL_H264_B1ListSwap(u4InstID, fgSLTIsFrmPic(u4InstID));
  }
}


VOID vAVC_SLT_AssignQuantParam(UINT32 u4InstID, VDEC_INFO_DEC_PRM_T *tVDecPrm)
{
  INT32 i;
  CHAR *ptList[8];

  for(i=0; i<8; i++)
  {
    tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i] = FALSE;
  }
   
  if((!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgPicScalingMatrixPresentFlag) 
    && (!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgSeqScalingMatrixPresentFlag))
  {
    tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingMatrixPresentFlag = FALSE;
    // do nothing
  }
  else
  {
    tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingMatrixPresentFlag = TRUE;
    if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgSeqScalingMatrixPresentFlag) // check sps first
    {
      for(i=0; i<8; i++)
      {
        tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i] = TRUE;
        if(i<6)
        {
          if(!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgSeqScalingListPresentFlag[i]) // fall-back rule A
          {
            if((i==0) || (i==3))
            {
              ptList[i] =  (i==0) ? quant_intra_default:quant_inter_default;
              vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);
            }
            else
            {
              ptList[i] =  ptList[i-1];
              tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i] = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i-1];
              //if(ptVerMpvDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i])
              {
                vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);
              }
            }
          }
          else // fall-back rule A
          {
            if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgUseDefaultScalingMatrix4x4Flag[i])
            {
              ptList[i] = (i<3) ? quant_intra_default:quant_inter_default;
              vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);
            }
            else
            {
              tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i] = TRUE;
              ptList[i] = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->cScalingList4x4[i];
              vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);           
            }
          }
        }
        else
        {
          if(!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgSeqScalingListPresentFlag[i] 
            || tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgUseDefaultScalingMatrix8x8Flag[i-6]) // fall-back rule A
          {
            ptList[i] = (i==6) ? quant8_intra_default:quant8_inter_default;
            vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);
          }
          else
          {
            tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i] = TRUE;
            ptList[i] = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->cScalingList8x8[i-6];
            vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);
          }
        }
      }
    }
    
    if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgPicScalingMatrixPresentFlag) // then check pps
    {
      for(i=0; i<8; i++)
      {
        if(i<6)
        {
          if(!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgPicScalingListPresentFlag[i]) // fall-back rule A
          {
            if((i==0) || (i==3))
            {              
              if(!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgSeqScalingMatrixPresentFlag)
              {
                ptList[i] = (i==0) ? quant_intra_default:quant_inter_default;
                //ptVerMpvDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i] = FALSE;
                vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);
              }
            }
            else
            {
              ptList[i] = ptList[i-1];
              tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i] = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i-1];
              //if(ptVerMpvDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i])
              {
                vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);
              }
            }
          }
          else
          {
            if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgUseDefaultScalingMatrix4x4Flag[i])
            {
              ptList[i] = (i<3) ? quant_intra_default:quant_inter_default;
              //ptVerMpvDecPrm->fgUserScalingListPresentFlag[i] = FALSE;
              vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);
            }
            else
            {
              ptList[i] = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->cScalingList4x4[i];
              tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i] = TRUE;
              vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);
            }
          }
        }
        else
        {
          if(!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgPicScalingListPresentFlag[i]) // fall-back rule B
          {
            if(!tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgSeqScalingMatrixPresentFlag)
            {
              ptList[i] = (i==6) ? quant8_intra_default:quant8_inter_default;
              //ptVerMpvDecPrm->fgUserScalingListPresentFlag[i] = FALSE;
              vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);
            }
          }
          else if(tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgUseDefaultScalingMatrix8x8Flag[i-6])
          {
            ptList[i] = (i==6) ? quant8_intra_default:quant8_inter_default;
            //ptVerMpvDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i] = FALSE;
            vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);
          }
          else
          {
            ptList[i] = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->cScalingList8x8[i-6];
            tVDecPrm->SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[i] = TRUE;
            vVDEC_HAL_H264_WriteScalingList(u4InstID,i, ptList[i]);       
          }
        }
      }
    }
  }
}


VOID vAVC_SLT_PrepareRefPiclist(UINT32 u4InstID, VDEC_INFO_DEC_PRM_T *tVDecPrm)
{
    vAVC_SLT_PrepareFBufInfo(u4InstID, tVDecPrm);
    vAVC_SLT_VDecSetPRefPicList(u4InstID);
    vAVC_SLT_VDecSetBRefPicList(u4InstID);
}

BOOL fgAVC_SLT_VDecComplete(UINT32 u4InstID)
{
    UINT32 u4MbX, u4MbY;

    vVDEC_HAL_H264_GetMbxMby(u4InstID, &u4MbX, &u4MbY);
    
    if(fgSLTIsFrmPic(u4InstID))
    {
        if(u4MbX < ((_ptCurrFBufInfo[u4InstID]->u4W >> 4) -1) || (u4MbY < ((_ptCurrFBufInfo[u4InstID]->u4H >> 4) -1)))
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        if(u4MbX < ((_ptCurrFBufInfo[u4InstID]->u4W >> 4) -1) || u4MbY < ((_ptCurrFBufInfo[u4InstID]->u4H >> 5) -1))
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    return FALSE;
}

void vAVC_SLT_VerifyClrFBufInfo(UINT32 u4InstID, UINT32 u4FBufIdx)
{
  _ptFBufInfo[u4InstID][u4FBufIdx].fgNonExisting = FALSE;    
  _ptFBufInfo[u4InstID][u4FBufIdx].eH264DpbStatus = H264_DPB_STATUS_EMPTY;
  _ptFBufInfo[u4InstID][u4FBufIdx].ucFBufStatus = NO_PIC;    
  
  _ptFBufInfo[u4InstID][u4FBufIdx].ucBFldRefType = NREF_PIC;    
  _ptFBufInfo[u4InstID][u4FBufIdx].ucFBufRefType = NREF_PIC;
  _ptFBufInfo[u4InstID][u4FBufIdx].ucTFldRefType = NREF_PIC;
  
  _ptFBufInfo[u4InstID][u4FBufIdx].u4FrameNum = 0xffffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4FrameNumWrap = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4PicNum = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4TFldPicNum = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4BFldPicNum = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].u4LongTermFrameIdx = 0xffffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].u4TFldLongTermFrameIdx = 0xffffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].u4BFldLongTermFrameIdx = 0xffffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4LongTermPicNum = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4TFldLongTermPicNum = 0xefffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4BFldLongTermPicNum = 0xefffffff;    

  _ptFBufInfo[u4InstID][u4FBufIdx].ucFBufStatus = 0;
  _ptFBufInfo[u4InstID][u4FBufIdx].u4TFldPara = 0;
  _ptFBufInfo[u4InstID][u4FBufIdx].u4BFldPara = 0;
  
  
  _ptFBufInfo[u4InstID][u4FBufIdx].i4POC = 0x7fffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4TFldPOC = 0x7fffffff;
  _ptFBufInfo[u4InstID][u4FBufIdx].i4BFldPOC = 0x7fffffff;
}


VOID vAVC_SLT_VerifyFlushBufInfo(UINT32 u4InstID)
{
    UINT32 i;
    
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.u4MaxLongTermFrameIdx = 0xffffffff;
    for(i = 0; i < 17; i++)
    {
        _ptFBufInfo[u4InstID][i].u4DecOrder = 0;
        vAVC_SLT_VerifyClrFBufInfo(u4InstID, i);
    }
    for(i=0; i<6; i++)
    {
        _ptRefPicList[u4InstID][i].u4RefPicCnt = 0;
    }

}
VOID vAVC_SLT_VerifyFlushAllSetData(UINT32 u4InstID)
{
    UINT32 i;

    for (i= 0; i<32; i++)
    {
        _rH264SPS[u4InstID][i].fgSPSValid = FALSE;
    }
    for (i=0; i<256; i++)
    {
        _rH264PPS[u4InstID][i].fgPPSValid = FALSE;
    }
}

VOID vAVC_SLT_VerifyFlushBufRefInfo(UINT32 u4InstID)
{
  UINT32 i;

  _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.u4MaxLongTermFrameIdx = 0xffffffff;
  
  for(i=0; i<17; i++)
  {
    _ptFBufInfo[u4InstID][i].fgNonExisting = FALSE;        
    _ptFBufInfo[u4InstID][i].ucFBufRefType = NREF_PIC;
    _ptFBufInfo[u4InstID][i].ucTFldRefType = NREF_PIC;
    _ptFBufInfo[u4InstID][i].ucBFldRefType = NREF_PIC;    
    _ptFBufInfo[u4InstID][i].u4LongTermFrameIdx = 0xffffffff;
    _ptFBufInfo[u4InstID][i].u4TFldLongTermFrameIdx = 0xffffffff;
    _ptFBufInfo[u4InstID][i].u4BFldLongTermFrameIdx = 0xffffffff;
  }
  for(i=0; i<3; i++)
  {        
    _ptRefPicList[u4InstID][i].u4RefPicCnt = 0;
  } 
}


void vAVC_SLT_VerifyClrPicRefInfo(UINT32 u4InstID, UCHAR ucPicType, UCHAR ucFBufIdx)
{
  if(ucPicType & TOP_FIELD)
  {
    _ptFBufInfo[u4InstID][ucFBufIdx].ucTFldRefType = NREF_PIC;
  }
  if(ucPicType & BOTTOM_FIELD)
  {
    _ptFBufInfo[u4InstID][ucFBufIdx].ucBFldRefType = NREF_PIC;
  }
  _ptFBufInfo[u4InstID][ucFBufIdx].ucFBufRefType = NREF_PIC;
}


VOID vAVC_SLT_VerifyDec_Ref_Pic_Marking(UINT32 u4InstID, VDEC_INFO_H264_SLICE_HDR_T *prSliceHdr)
{   
  UINT32 u4Cnt;
  if(fgSLTIsIDRPic(u4InstID))
  {
    prSliceHdr->fgNoOutputOfPriorPicsFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    prSliceHdr->fgLongTermReferenceFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  }
  else
  {
    prSliceHdr->fgAdaptiveRefPicMarkingModeFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(prSliceHdr->fgAdaptiveRefPicMarkingModeFlag)
    {
      u4Cnt = 0;
      do
      {
        prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
        if((prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] == 1))
        {
          prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] |= (u4VDEC_HAL_H264_UeCodeNum(0, u4InstID) << 8);  
        }
        else if(prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] == 2)
        {
          prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] |= (u4VDEC_HAL_H264_UeCodeNum(0, u4InstID) << 8);  
        }
        else if(prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] == 3)
        {
          prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] |= (u4VDEC_HAL_H264_UeCodeNum(0, u4InstID) << 8);  
          prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] |= (u4VDEC_HAL_H264_UeCodeNum(0, u4InstID) << 16);         
        }
        else if(prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] == 4)
        {
          prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] |= (u4VDEC_HAL_H264_UeCodeNum(0, u4InstID) << 8);                     
        }
        else if(prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] == 5)
        {
          prSliceHdr->fgMmco5 = TRUE;
        }
        else if(prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] == 6)
        {
          prSliceHdr->u4MemoryManagementControlOperation[u4Cnt] |= (u4VDEC_HAL_H264_UeCodeNum(0, u4InstID) << 8);           
        }        
        u4Cnt ++;
      }while(prSliceHdr->u4MemoryManagementControlOperation[u4Cnt-1] != 0);
    }
  }
}

VOID vAVC_SLT_VerifyRef_Pic_List_Reordering(UINT32 u4InstID, VDEC_INFO_H264_SLICE_HDR_T *prSliceHdr)
{
    if (!fgIsISlice(prSliceHdr->u4SliceType))
    {
        vVDEC_HAL_H264_Reording(u4InstID);
    }
}

BOOL vH264SPSParamConfig(VDEC_INFO_H264_SPS_T *prSPS,VDEC_HAL_H264_SPS_T *prHALSPS)
{
    if((prSPS == NULL) || (prHALSPS == NULL))
    {
       // UTIL_Printf("Error reference NULL pointer @ %s!\n",__FUNCTION__);
        return FALSE;
    }

    prHALSPS->fgDeltaPicOrderAlwaysZeroFlag     = prSPS->fgDeltaPicOrderAlwaysZeroFlag;
    prHALSPS->fgDirect8x8InferenceFlag          = prSPS->fgDirect8x8InferenceFlag;
    prHALSPS->fgFrameMbsOnlyFlag                = prSPS->fgFrameMbsOnlyFlag;
    prHALSPS->fgMbAdaptiveFrameFieldFlag        = prSPS->fgMbAdaptiveFrameFieldFlag;
    prHALSPS->u4Log2MaxFrameNumMinus4           = prSPS->u4Log2MaxFrameNumMinus4;
    prHALSPS->u4Log2MaxPicOrderCntLsbMinus4     = prSPS->u4Log2MaxPicOrderCntLsbMinus4;
    prHALSPS->u4NumRefFrames                    = prSPS->u4NumRefFrames;
    prHALSPS->u4PicHeightInMapUnitsMinus1       = prSPS->u4PicHeightInMapUnitsMinus1;
    prHALSPS->u4PicOrderCntType                 = prSPS->u4PicOrderCntType;
    prHALSPS->u4PicWidthInMbsMinus1             = prSPS->u4PicWidthInMbsMinus1;
    prHALSPS->u4ChromaFormatIdc                 = prSPS->u4ChromaFormatIdc;
    return TRUE;
}

BOOL vH264PPSParamConfig(VDEC_INFO_H264_PPS_T *prPPS,VDEC_HAL_H264_PPS_T *prHALPPS)
{
    if((prPPS == NULL) || (prHALPPS == NULL))
    {
        //UTIL_Printf("Error reference NULL pointer @ %s!\n",__FUNCTION__);
        return FALSE;
    }

    prHALPPS->fgConstrainedIntraPredFlag            = prPPS->fgConstrainedIntraPredFlag;
    prHALPPS->fgDeblockingFilterControlPresentFlag  = prPPS->fgDeblockingFilterControlPresentFlag;
    prHALPPS->fgEntropyCodingModeFlag               = prPPS->fgEntropyCodingModeFlag;
    prHALPPS->fgPicOrderPresentFlag                 = prPPS->fgPicOrderPresentFlag;
    prHALPPS->fgTransform8x8ModeFlag                = prPPS->fgTransform8x8ModeFlag;
    prHALPPS->fgWeightedPredFlag                    = prPPS->fgWeightedPredFlag;
    prHALPPS->i4ChromaQpIndexOffset                 = prPPS->i4ChromaQpIndexOffset;
    prHALPPS->i4PicInitQpMinus26                    = prPPS->i4PicInitQpMinus26;
    prHALPPS->i4SecondChromaQpIndexOffset           = prPPS->i4SecondChromaQpIndexOffset;
    prHALPPS->u4NumRefIdxL0ActiveMinus1             = prPPS->u4NumRefIdxL0ActiveMinus1;
    prHALPPS->u4NumRefIdxL1ActiveMinus1             = prPPS->u4NumRefIdxL1ActiveMinus1;
    prHALPPS->u4WeightedBipredIdc                   = prPPS->u4WeightedBipredIdc;

    return TRUE;
}

BOOL vH264SliceParamConfig(VDEC_INFO_H264_SLICE_HDR_T *prSliceHdr,VDEC_HAL_H264_SLICE_HDR_T *prHALSliceHdr)
{
    if((prSliceHdr == NULL) || (prHALSliceHdr == NULL))
    {
        //UTIL_Printf("Error reference NULL pointer @ %s!\n",__FUNCTION__);
        return FALSE;
    }

    prHALSliceHdr->fgBottomFieldFlag            = prSliceHdr->fgBottomFieldFlag;
    prHALSliceHdr->fgDirectSpatialMvPredFlag    = prSliceHdr->fgDirectSpatialMvPredFlag;
    prHALSliceHdr->fgFieldPicFlag               = prSliceHdr->fgFieldPicFlag;
    prHALSliceHdr->u4FirstMbInSlice             = prSliceHdr->u4FirstMbInSlice;
    prHALSliceHdr->u4NumRefIdxL0ActiveMinus1    = prSliceHdr->u4NumRefIdxL0ActiveMinus1;
    prHALSliceHdr->u4NumRefIdxL1ActiveMinus1    = prSliceHdr->u4NumRefIdxL1ActiveMinus1;
    prHALSliceHdr->u4SliceType                  = prSliceHdr->u4SliceType;
    return TRUE;
}

VOID vAVC_SLT_VerifyVDecSetPicInfo(UINT32 u4InstID, VDEC_INFO_DEC_PRM_T *ptVDecPrm)
{
	VDEC_HAL_H264_DEC_PRM_T *prH264HalPrm;
  prH264HalPrm = VDec_GetH264HalParam(u4InstID);

    vAVC_SLT_PrepareRefPiclist(u4InstID, ptVDecPrm);

    vAVC_SLT_AssignQuantParam(u4InstID, ptVDecPrm);
    
    vH264SPSParamConfig(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS,&prH264HalPrm->rSPSHALParam);
    vH264PPSParamConfig(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS,&prH264HalPrm->rPPSHALParam);
    vH264SliceParamConfig(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr,&prH264HalPrm->rSliceHALParam);
    
    vVDEC_HAL_H264_SetSPSAVLD(u4InstID, &prH264HalPrm->rSPSHALParam);
    vVDEC_HAL_H264_SetPPSAVLD(u4InstID,  _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.fgUserScalingMatrixPresentFlag,
        &(_tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.fgUserScalingListPresentFlag[0]), &prH264HalPrm->rPPSHALParam);
    vVDEC_HAL_H264_SetSHDRAVLD1(u4InstID, &prH264HalPrm->rSliceHALParam);
}

VOID vAVC_SLT_VerifyInitSliceHdr(VDEC_INFO_DEC_PRM_T *tVDecPrm)//open verify config
{
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[0] = 0;
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4DeltaPicOrderCnt[1] = 0;
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4RedundantPicCnt = 0;  
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4DisableDeblockingFilterIdc = 0;  
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4SliceAlphaC0OffsetDiv2 = 0;    
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->i4SliceBetaOffsetDiv2 = 0;    
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4NumRefIdxL0ActiveMinus1 = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->u4NumRefIdxL0ActiveMinus1;
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4NumRefIdxL1ActiveMinus1 = tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->u4NumRefIdxL1ActiveMinus1;
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4LongTermFrameIdx = 0xffffffff; 
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4LongTermPicNum = 0;  
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgDirectSpatialMvPredFlag = FALSE;
  tVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->u4IdrPicId = 0;
}

VOID vAVC_SLT_InterpretBufferingPeriodInfo(UINT32 u4InstID, VDEC_INFO_DEC_PRM_T *ptVDecPrm)
{
  UINT32 u4SPSID;
  UINT32 k;
  
  u4SPSID = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  if(_rH264SPS[u4InstID][u4SPSID].fgSPSValid)
  {
    if(( (!ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr)
          || !ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr->fgNoOutputOfPriorPicsFlag) 
       &&  (ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS != (&_rH264SPS[u4InstID][u4SPSID])))
    {
      vAVC_SLT_VerifyFlushBufRefInfo(u4InstID);
    }
    ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS = &_rH264SPS[u4InstID][u4SPSID];
    if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgVuiParametersPresentFlag)
    {
      if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->rVUI.fgNalHrdParametersPresentFlag)
      {
        for (k=0; k<ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->rVUI.tNalHrdParameters.u4CpbCntMinus1+1; k++)
        {
          u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->rVUI.tNalHrdParameters.u4InitialCpbRemovalDelayLengthMinus1 + 1);
          u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->rVUI.tNalHrdParameters.u4InitialCpbRemovalDelayLengthMinus1 + 1);
        }
      }
        
      if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->rVUI.fgVclHrdParametersPresentFlag)
      {
        for (k=0; k<ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->rVUI.tVclHrdParameters.u4CpbCntMinus1+1; k++)
        {
          u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->rVUI.tVclHrdParameters.u4InitialCpbRemovalDelayLengthMinus1 + 1);
          u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->rVUI.tVclHrdParameters.u4InitialCpbRemovalDelayLengthMinus1 + 1);
        }
      }
    }
  }
}

//#endif
VOID vAVC_SLT_InterpretFilmGrainCharacteristicsInfo(UINT32 u4InstID, VDEC_INFO_DEC_PRM_T *ptVDecPrm)
{
  UINT32 c,i,j,k;
  UINT32 u4Temp;

  ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->fgFilmGrainCharacteristicsCancelFlag  = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID); // Used to shift 1-bit
  if(!ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->fgFilmGrainCharacteristicsCancelFlag)
  {
    ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4ModelId = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 2);
    
    ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->fgSeparateColourDescriptionPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->fgSeparateColourDescriptionPresentFlag)
    {
      ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4FilmGrainBitDepthLumaMinus8 = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 3);
      ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4FilmGrainBitDepthChromaMinus8 = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 3);
      ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->fgFilmGrainFullRangeFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
      ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4FilmGrainColourPrimaries = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 8);
      ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4FilmGrainTransferCharacteristics = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 8);
      ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4FilmGrainMatrixCoefficients = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 8);
    }
    ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4BlendingModeId = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 2);
    ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4Log2ScaleFactor = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 4);
    for(c=0; c<3; c++)
    {
      ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->fgCompModelPresentFlag[c] = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    }
    for(c=0; c<3; c++)
    {
      if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->fgCompModelPresentFlag[c])
      {
        ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4NumIntensityIntervalsMinus1[c] = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 8);
        ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4NumModelValuesMinus1[c] = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 3);
        for(i=0; i<256; i++) // Initialize
        {
          ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->pucCompModelValue[(c << 9) + (i << 1)] = 0;
          ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->pucCompModelValue[(c << 9) + (i << 1) + 1] = 0;
        }
        for(i=0; i<=ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4NumIntensityIntervalsMinus1[c]; i++)        
        {
          ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4IntensityIntervalLowerBound[c][i] = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 8);
          ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4IntensityIntervalUpperBound[c][i] = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 8);
          for(j=0; j<=ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4NumModelValuesMinus1[c]; j++)        //0,1,2
          {
            u4Temp = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
            if(j == 0)
            {
              for(k=ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4IntensityIntervalLowerBound[c][i]; k<=ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4IntensityIntervalUpperBound[c][i]; k++)
              {
                ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->pucCompModelValue[(c << 9) + (k << 1)] = (u4Temp & 0xff);
              }
            }
            else if(j == 1)
            {
              for(k=ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4IntensityIntervalLowerBound[c][i]; k<=ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4IntensityIntervalUpperBound[c][i]; k++)
              {
                ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->pucCompModelValue[(c << 9) + (k << 1) + 1] = (ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->pucCompModelValue[(c << 9) + (k << 1) + 1] & 0xf0) + (u4Temp & 0xf);
              }
            }
            else if(j == 2)
            {
              for(k=ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4IntensityIntervalLowerBound[c][i]; k<=ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4IntensityIntervalUpperBound[c][i]; k++)
              {
                ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->pucCompModelValue[(c << 9) + (k << 1) + 1] = (ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->pucCompModelValue[(c << 9) + (k << 1) + 1] & 0xf) + ((u4Temp & 0xf) << 4);
              }
            }
          }
        }
      }      
    }
    ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSEI->u4FilmGrainCharacteristicsRepetitionPeriod = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);      
  }
}

VOID vAVS_SLT_VerifyInitSPS(VDEC_INFO_H264_SPS_T *prSPS)
{
    INT32 i;

    prSPS->u4ChromaFormatIdc = 1;
    prSPS->u4BitDepthLumaMinus8 = 0;
    prSPS->u4BitDepthChromaMinus8 = 0;
    prSPS->fgQpprimeYZeroTransformBypassFlag = FALSE;
    prSPS->fgSeqScalingMatrixPresentFlag = FALSE;
    for(i=0; i<8; i++)
    {
        prSPS->fgSeqScalingListPresentFlag[i] = FALSE;
    }
}

void vAVS_SLT_VerifyHrdParameters(UINT32 u4InstID, VDEC_INFO_H264_HRD_PRM_T *tHrdPara)
{
  UINT32 u4SchedSelIdx;
  
  tHrdPara->u4CpbCntMinus1 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  tHrdPara->u4BitRateScale = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 4);
  tHrdPara->u4CpbSizeScale = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 4);  
  for(u4SchedSelIdx=0; u4SchedSelIdx<=  tHrdPara->u4CpbCntMinus1; u4SchedSelIdx++)
  {
    tHrdPara->u4BitRateValueMinus1[u4SchedSelIdx] = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    tHrdPara->u4CpbSizeValueMinus1[u4SchedSelIdx] = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    tHrdPara->fgCbrFlag[u4SchedSelIdx] = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  }
  tHrdPara->u4InitialCpbRemovalDelayLengthMinus1 = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 5);  
  tHrdPara->u4CpbRemovalDelayLengthMinus1 = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 5);  
  tHrdPara->u4DpbOutputDelayLengthMinus1 = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 5);  
  tHrdPara->u4TimeOffsetLength = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 5);  
}

VOID vAVC_SLT_VerifyPic_Par_Set_Rbsp(UINT32 u4InstID)
{
  UINT32 i;
  UINT32 u4PPSID;
  VDEC_INFO_H264_SPS_T *prSPS;
  VDEC_INFO_H264_PPS_T *ptPPS;

  u4PPSID = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  if(u4PPSID < 256)
  {
    ptPPS = &_rH264PPS[u4InstID][u4PPSID];
    ptPPS->fgPPSValid = FALSE; // FALSE until set completely
  }
  else
  {
    Printf("[AVC_SLT]err in PPS Num err\\n\\0");
    return;
  }

  ptPPS->u4SeqParameterSetId = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  prSPS = &_rH264SPS[u4InstID][ptPPS->u4SeqParameterSetId];


  ptPPS->fgEntropyCodingModeFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  ptPPS->fgPicOrderPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  ptPPS->u4NumSliceGroupsMinus1 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);

  if(ptPPS->u4NumSliceGroupsMinus1 > 255)
  {
    Printf("///Warning!!! num_slice_groups_minus1 size isn't enough ///\n");
  }

  if(ptPPS->u4NumSliceGroupsMinus1 > 0)
  {
    ptPPS->u4SliceGroupMapType = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);

    if(ptPPS->u4SliceGroupMapType == 0)
    {
    	for(i = 0; i <= ptPPS->u4NumSliceGroupsMinus1; i++)
    	{
    		ptPPS->u4RunLengthMinus1[i] = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    	}
    }
    else if(ptPPS->u4SliceGroupMapType == 2)
    {
      for(i = 0; i < ptPPS->u4NumSliceGroupsMinus1; i++)
      {
        ptPPS->u4TopLeft[i] = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
        ptPPS->u4BottomRight[i] = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
      }
    }
    else if((ptPPS->u4SliceGroupMapType == 3) ||
      	        (ptPPS->u4SliceGroupMapType == 4) ||
    		     (ptPPS->u4SliceGroupMapType == 5))
    {
      ptPPS->fgSliceGroupChangeDirectionFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
      ptPPS->u4SliceGroupChangeRateMinus1 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    }
    else if(ptPPS->u4SliceGroupMapType == 6)
    {
      ptPPS->u4PicSizeInMapUnitsMinus1 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
      for(i = 0; i <= ptPPS->u4PicSizeInMapUnitsMinus1; i++)
      {
            ptPPS->au4SliceGroupId[i] = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
      }
    }
  }

  ptPPS->u4NumRefIdxL0ActiveMinus1 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  ptPPS->u4NumRefIdxL1ActiveMinus1 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  ptPPS->fgWeightedPredFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  ptPPS->u4WeightedBipredIdc = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 2); 
  ptPPS->i4PicInitQpMinus26 = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
  ptPPS->i4PicInitQsMinus26 = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
  ptPPS->i4ChromaQpIndexOffset = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
  ptPPS->fgDeblockingFilterControlPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  ptPPS->fgConstrainedIntraPredFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  ptPPS->fgRedundantPicCntPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);

  if(bVDEC_HAL_H264_IsMoreRbspData(0, u4InstID))
  {
    ptPPS->fgTransform8x8ModeFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    ptPPS->fgPicScalingMatrixPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(ptPPS->fgPicScalingMatrixPresentFlag)
    {
      for(i=0; i<(UINT32)((ptPPS->fgTransform8x8ModeFlag << 1) + 6); i++)
      {
        ptPPS->fgPicScalingListPresentFlag[i] = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
        if(ptPPS->fgPicScalingListPresentFlag[i])
        {
          if(i < 6)
          {
            vVDEC_HAL_H264_ScalingList(0, u4InstID, ptPPS->cScalingList4x4[i],16, &ptPPS->fgUseDefaultScalingMatrix4x4Flag[i]);
          }
          else
          {
            vVDEC_HAL_H264_ScalingList(0, u4InstID, ptPPS->cScalingList8x8[i-6], 64, &ptPPS->fgUseDefaultScalingMatrix8x8Flag[i-6]);
          }
        }
      }
    }
    ptPPS->i4SecondChromaQpIndexOffset = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
  }
  else
  {
    ptPPS->fgTransform8x8ModeFlag = 0;
    ptPPS->fgPicScalingMatrixPresentFlag = FALSE;
    ptPPS->i4SecondChromaQpIndexOffset = ptPPS->i4ChromaQpIndexOffset;
  }
  vVDEC_HAL_H264_TrailingBits(0, u4InstID);

  ptPPS->fgPPSValid = TRUE;
}


VOID vAVC_SLT_VerifySeq_Par_Set_Rbsp(UINT32 u4InstID)
{
  UINT32 u4Temp;
  UINT32 i;
  UINT32 u4SeqParameterSetId;
  VDEC_INFO_H264_SPS_T *prSPS = NULL;

  u4Temp = (u4VDEC_HAL_H264_GetBitStreamShift(0, u4InstID, 24) >> 8);

  if((u4Temp & 0xf00) > 0)  // [11:8]
  {
    Printf("err in SPS Forbiden Zero\n");
    return;
  }

  // 1st
  u4SeqParameterSetId = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  if(u4SeqParameterSetId < 32)
  {   
    prSPS = &_rH264SPS[u4InstID][u4SeqParameterSetId];
    prSPS->fgSPSValid = TRUE;

    prSPS->u4ProfileIdc = (u4Temp >> 16);                              // [23:16]
    prSPS->fgConstrainedSet0Flag = (u4Temp >> 15) & 0x1;      // [15]
    prSPS->fgConstrainedSet1Flag = (u4Temp >> 14) & 0x1;      // [14]
    prSPS->fgConstrainedSet2Flag = (u4Temp >> 13) & 0x1;      // [13]
    prSPS->fgConstrainedSet3Flag = (u4Temp >> 12) & 0x1;      // [12]
    prSPS->fgConstrainedSet4Flag = (u4Temp >> 11) & 0x1;      // [11] According to the latest spec AD007
    prSPS->u4LevelIdc = (u4Temp & 0xff);                              // [7:0]
    if(prSPS->fgConstrainedSet3Flag && (prSPS->u4LevelIdc == 11))
    {
        // should be 1b
        prSPS->u4LevelIdc = 10;
    }
    prSPS->u4SeqParameterSetId = u4SeqParameterSetId;
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.rLastInfo.ucLastSPSId = prSPS->u4SeqParameterSetId;
  }
  else
  {
    Printf("err in SPS Num\n");
  }
  
  vAVS_SLT_VerifyInitSPS(prSPS);
  
  // 2nd
  if((prSPS->u4ProfileIdc == FREXT_HP_PROFILE) 
    || (prSPS->u4ProfileIdc == FREXT_Hi10P_PROFILE) 
    || (prSPS->u4ProfileIdc == FREXT_Hi422_PROFILE)
    || (prSPS->u4ProfileIdc == FREXT_Hi444_PROFILE))
  {
    prSPS->u4ChromaFormatIdc = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    if(prSPS->u4ChromaFormatIdc == 3)
    {
        prSPS->fgResidualColorTransformFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    }
    prSPS->u4BitDepthLumaMinus8 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    prSPS->u4BitDepthChromaMinus8 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    prSPS->fgQpprimeYZeroTransformBypassFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    prSPS->fgSeqScalingMatrixPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    
    if(prSPS->fgSeqScalingMatrixPresentFlag)
    {
      for(i = 0; i < 8; i ++)
      {
        prSPS->fgSeqScalingListPresentFlag[i] = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
        if(prSPS->fgSeqScalingListPresentFlag[i])
        {
          if(i < 6)
          {
            vVDEC_HAL_H264_ScalingList(0, u4InstID, prSPS->cScalingList4x4[i],16, &prSPS->fgUseDefaultScalingMatrix4x4Flag[i]);
          }
          else
          {
            vVDEC_HAL_H264_ScalingList(0, u4InstID, prSPS->cScalingList8x8[i-6],64, &prSPS->fgUseDefaultScalingMatrix8x8Flag[i-6]);
          }
        }
      }
    }
  }

  prSPS->u4Log2MaxFrameNumMinus4 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  prSPS->u4MaxFrameNum = 2 << (prSPS->u4Log2MaxFrameNumMinus4 + 4 - 1); 
  prSPS->u4PicOrderCntType = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  if(prSPS->u4PicOrderCntType == 0)
  {
    prSPS->u4Log2MaxPicOrderCntLsbMinus4 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  }
  else if(prSPS->u4PicOrderCntType == 1)
  {
    prSPS->fgDeltaPicOrderAlwaysZeroFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    prSPS->i4OffsetForNonRefPic = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
    prSPS->i4OffsetForTopToBottomField = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
    prSPS->u4NumRefFramesInPicOrderCntCycle = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    for(i=0 ; i<prSPS->u4NumRefFramesInPicOrderCntCycle; i++)
    {
      prSPS->i4OffsetForRefFrame[i] = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
    }
  }
  
  prSPS->u4NumRefFrames = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  prSPS->fgGapsInFrameNumValueAllowedFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  prSPS->u4PicWidthInMbsMinus1 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  prSPS->u4PicHeightInMapUnitsMinus1 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  prSPS->fgFrameMbsOnlyFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  
  if((prSPS->u4ProfileIdc >= 77) // upper than Main Profile
      && ((prSPS->u4LevelIdc <= 20) || (prSPS->u4LevelIdc >= 42)))
  {
    prSPS->fgFrameMbsOnlyFlag = 1;
  }

  prSPS->fgMbAdaptiveFrameFieldFlag = FALSE;
  if(!prSPS->fgFrameMbsOnlyFlag)
  {
    prSPS->fgMbAdaptiveFrameFieldFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  }
  prSPS->fgDirect8x8InferenceFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  if((prSPS->u4ProfileIdc >= 77) // upper than Main Profile
      && (prSPS->u4LevelIdc >= 30))
  {
    prSPS->fgDirect8x8InferenceFlag = 1;
  }  

  prSPS->fgFrameCroppingFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);

  if(prSPS->fgFrameCroppingFlag)
  {
    prSPS->u4FrameCropLeftOffset = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    prSPS->u4FrameCropRightOffset = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    prSPS->u4FrameCropTopOffset = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    prSPS->u4FrameCropBottomOffset = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  }
  else
  {
    prSPS->u4FrameCropLeftOffset = 0;
    prSPS->u4FrameCropRightOffset = 0;
    prSPS->u4FrameCropTopOffset = 0;
    prSPS->u4FrameCropBottomOffset = 0;
  }

  prSPS->fgVuiParametersPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
#if 1
  if(prSPS->fgVuiParametersPresentFlag)
  {
    prSPS->rVUI.fgAspectRatioInfoPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(prSPS->rVUI.fgAspectRatioInfoPresentFlag)
    {
      prSPS->rVUI.u4AspectRatioIdc = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 8);
      if(prSPS->rVUI.u4AspectRatioIdc == 255) //Extended_SAR
      {
        prSPS->rVUI.u4SarWidth = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 16);
        prSPS->rVUI.u4SarHeight = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 16);
      }
    }
    prSPS->rVUI.fgOverscanInfoPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(prSPS->rVUI.fgOverscanInfoPresentFlag)
    {
      prSPS->rVUI.fgOverscanAppropriateFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    }
    prSPS->rVUI.fgVideoSignalTypePresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(prSPS->rVUI.fgVideoSignalTypePresentFlag)
    {
      prSPS->rVUI.u4VideoFormat = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 3); 
      prSPS->rVUI.fgVideoFullRangeFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
      prSPS->rVUI.fgColourDescriptionPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
      if(prSPS->rVUI.fgColourDescriptionPresentFlag)
      {
        prSPS->rVUI.u4ColourPrimaries = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 8); 
        prSPS->rVUI.u4TransferCharacteristics = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 8); 
        prSPS->rVUI.u4MatrixCoefficients = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 8); 
      }
    }
    prSPS->rVUI.fgChromaLocationInfoPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(prSPS->rVUI.fgChromaLocationInfoPresentFlag)
    {
      prSPS->rVUI.u4ChromaSampleLocTypeTopField = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID); 
      prSPS->rVUI.u4ChromaSampleLocTypeBottomField = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID); 
    }
    prSPS->rVUI.fgTimingInfoPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(prSPS->rVUI.fgTimingInfoPresentFlag)
    {
      prSPS->rVUI.u4NumUnitsInTick = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 32); 
      prSPS->rVUI.u4TimeScale = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, 32); 
      prSPS->rVUI.fgFixedFrameRateFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    }
    prSPS->rVUI.fgNalHrdParametersPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(prSPS->rVUI.fgNalHrdParametersPresentFlag)
    {
      vAVS_SLT_VerifyHrdParameters(u4InstID, &prSPS->rVUI.tNalHrdParameters);
    }
    prSPS->rVUI.fgVclHrdParametersPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(prSPS->rVUI.fgVclHrdParametersPresentFlag)
    {
      vAVS_SLT_VerifyHrdParameters(u4InstID, &prSPS->rVUI.tVclHrdParameters);
    }
    if(prSPS->rVUI.fgNalHrdParametersPresentFlag || prSPS->rVUI.fgVclHrdParametersPresentFlag)
    {
      prSPS->rVUI.fgLowDelayHrdFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    }
    prSPS->rVUI.fgPicStructPresentFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    prSPS->rVUI.fgBitstreamRestrictionFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(prSPS->rVUI.fgBitstreamRestrictionFlag)
    {
      prSPS->rVUI.fgMotionVectorsOverPicBoundariesFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
      prSPS->rVUI.u4MaxBytesPerPicDenom = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
      prSPS->rVUI.u4MaxBitsPerMbDenom = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
      prSPS->rVUI.u4Log2MaxMvLengthHorizontal = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
      prSPS->rVUI.u4Log2MaxMvLengthVertical = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
      prSPS->rVUI.u4NumReorderFrames = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
      prSPS->rVUI.u4MaxDecFrameBuffering = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    }
  }
#endif

  vVDEC_HAL_H264_TrailingBits(0, u4InstID);
  
  prSPS->fgSPSValid = TRUE;
}

VOID vAVC_SLT_VerifySEI_Rbsp(UINT32 u4InstID)
{
  UINT32 u4PayloadType = 0;
  INT32 u4PayloadSize = 0;
  UINT32 u4Offset = 1;
  UCHAR bTmpByte = 0;
  do
  {
    // sei_message();
    u4PayloadType = 0;
    while ((bTmpByte = (u4VDEC_HAL_H264_GetBitStreamShift(0, u4InstID, 8) >> 24)) == 0xFF)
    {
      u4PayloadType += 255;
    }
    u4PayloadType += bTmpByte;   // this is the last UCHAR

    u4PayloadSize = 0;
    while ((bTmpByte = (u4VDEC_HAL_H264_GetBitStreamShift(0, u4InstID, 8) >> 24)) == 0xFF)
    {
      u4PayloadSize += 255;
    }
    u4PayloadSize += bTmpByte;   // this is the last UCHAR
    
    switch ( u4PayloadType )     // sei_payload( type, size );
    {
      case  SEI_BUFFERING_PERIOD:
        vAVC_SLT_InterpretBufferingPeriodInfo(u4InstID, &_tVDecPrm[u4InstID]);
        break;
      case  SEI_FILM_GRAIN_CHARACTERISTICS:
        vAVC_SLT_InterpretFilmGrainCharacteristicsInfo(u4InstID, &_tVDecPrm[u4InstID]);
        break;
      default:        
        //vInitVDecHW();
        while(u4PayloadSize > 0)
        {
          bTmpByte = u4VDEC_HAL_H264_ShiftGetBitStream(0, u4InstID, 8);
          u4PayloadSize --;
        }
        break;
    }
    u4Offset += u4PayloadSize;
    vVDEC_HAL_H264_TrailingBits(0, u4InstID);
  } while(bVDEC_HAL_H264_IsMoreRbspData(0, u4InstID));    // more_rbsp_data()  msg[offset] != 0x80
  // ignore the trailing bits rbsp_trailing_bits();  
}

VOID vAVC_SLT_ParseSliceHeader(UINT32 u4InstID)
{
  VDEC_INFO_H264_SLICE_HDR_T *prSliceHdr;
  VDEC_INFO_DEC_PRM_T *ptVDecPrm;
  UINT32 u4PPSID;
  UINT32 u4Temp;

  u4Temp = u4VDEC_HAL_H264_ShiftGetBitStream(0, u4InstID, 0);
  ptVDecPrm = &_tVDecPrm[u4InstID];
  prSliceHdr = &_rH264SliceHdr[u4InstID];
  ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSliceHdr = prSliceHdr;
 
  prSliceHdr->u4FirstMbInSlice = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  prSliceHdr->u4SliceType = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);

  u4PPSID = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  if((u4PPSID < 256)
      && (_rH264PPS[u4InstID][u4PPSID].fgPPSValid)
      && (_rH264SPS[u4InstID][_rH264PPS[u4InstID][u4PPSID].u4SeqParameterSetId].fgSPSValid))
  {
    ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS = &_rH264PPS[u4InstID][u4PPSID];
    ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS = &_rH264SPS[u4InstID][_rH264PPS[u4InstID][u4PPSID].u4SeqParameterSetId];
  }

  vAVC_SLT_VerifyInitSliceHdr(ptVDecPrm);
  prSliceHdr->u4FrameNum = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4Log2MaxFrameNumMinus4 + 4);    

  prSliceHdr->fgFieldPicFlag = FALSE;
  prSliceHdr->fgBottomFieldFlag = FALSE;
  if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgFrameMbsOnlyFlag)
  {
    ptVDecPrm->ucPicStruct = FRAME;
  }
  else
  {
    prSliceHdr->fgFieldPicFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(prSliceHdr->fgFieldPicFlag)
    {
      prSliceHdr->fgBottomFieldFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
      ptVDecPrm->ucPicStruct = (prSliceHdr->fgBottomFieldFlag) ? BOTTOM_FIELD : TOP_FIELD;            
    }
    else
    {
      ptVDecPrm->ucPicStruct = FRAME;           
    }
  }
  
  if(fgSLTIsIDRPic(u4InstID)) //IDR picture
  {
    prSliceHdr->u4IdrPicId = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  }
  if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4PicOrderCntType == 0)
  {
    prSliceHdr->i4PicOrderCntLsb = u4VDEC_HAL_H264_GetRealBitStream(0, u4InstID, ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4Log2MaxPicOrderCntLsbMinus4 + 4);
    if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgPicOrderPresentFlag && (!prSliceHdr->fgFieldPicFlag))
    {
      prSliceHdr->i4DeltaPicOrderCntBottom = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
    }
    else
    {
      prSliceHdr->i4DeltaPicOrderCntBottom = 0;
    }
  }
  
  if((ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4PicOrderCntType == 1) && (!ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->fgDeltaPicOrderAlwaysZeroFlag))
  {
    prSliceHdr->i4DeltaPicOrderCnt[0] = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
    if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgPicOrderPresentFlag && (!prSliceHdr->fgFieldPicFlag))
    {
      prSliceHdr->i4DeltaPicOrderCnt[1] = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
    }
  }
  else
  {
    if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prSPS->u4PicOrderCntType == 1)
    {
      prSliceHdr->i4DeltaPicOrderCnt[0] = 0;
      prSliceHdr->i4DeltaPicOrderCnt[1] = 0;
    }
  }
  
  if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgRedundantPicCntPresentFlag)
  {
    prSliceHdr->u4RedundantPicCnt = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  }
  if(fgIsBSlice(prSliceHdr->u4SliceType))
  {
    prSliceHdr->fgDirectSpatialMvPredFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  }
  if(fgIsPSlice(prSliceHdr->u4SliceType) || fgIsBSlice(prSliceHdr->u4SliceType))
  {
    prSliceHdr->fgNumRefIdxActiveOverrideFlag = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
    if(prSliceHdr->fgNumRefIdxActiveOverrideFlag)
    {
      prSliceHdr->u4NumRefIdxL0ActiveMinus1 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
      if(fgIsBSlice(prSliceHdr->u4SliceType))
      {
        prSliceHdr->u4NumRefIdxL1ActiveMinus1 = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
      }
    }
  }
  if(!fgIsBSlice(prSliceHdr->u4SliceType))
  {
    prSliceHdr->u4NumRefIdxL1ActiveMinus1 = 0;
  }

  vAVC_SLT_VerifyVDecSetPicInfo(u4InstID, ptVDecPrm);
  
  vAVC_SLT_VerifyRef_Pic_List_Reordering(u4InstID, prSliceHdr);

  if((ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgWeightedPredFlag && fgIsPSlice(prSliceHdr->u4SliceType))
    || ((ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->u4WeightedBipredIdc == 1) && fgIsBSlice(prSliceHdr->u4SliceType)))
  {
    vVDEC_HAL_H264_PredWeightTable(u4InstID);
  }

  if(_u4NalRefIdc[u4InstID] != 0)
  {
    prSliceHdr->fgMmco5 = FALSE;      
    vAVC_SLT_VerifyDec_Ref_Pic_Marking(u4InstID, prSliceHdr);
  }

  if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgEntropyCodingModeFlag && (!fgIsISlice(prSliceHdr->u4SliceType)))
  {
    prSliceHdr->u4CabacInitIdc = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
  }
  else
  {
    prSliceHdr->u4CabacInitIdc = 0;
  }

  prSliceHdr->i4SliceQpDelta = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);


  if(ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->fgDeblockingFilterControlPresentFlag)
  {
    prSliceHdr->u4DisableDeblockingFilterIdc = u4VDEC_HAL_H264_UeCodeNum(0, u4InstID);
    if(prSliceHdr->u4DisableDeblockingFilterIdc != 1)
    {
      prSliceHdr->i4SliceAlphaC0OffsetDiv2 = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
      prSliceHdr->i4SliceBetaOffsetDiv2 = i4VDEC_HAL_H264_SeCodeNum(0, u4InstID);
    }
    else
    {
      prSliceHdr->i4SliceAlphaC0OffsetDiv2 = 0;
      prSliceHdr->i4SliceBetaOffsetDiv2 = 0;
    }
  }

  if((ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->u4NumSliceGroupsMinus1 > 0) && 
     (ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->u4SliceGroupMapType >= 3) &&
     (ptVDecPrm->SpecDecPrm.rVDecH264DecPrm.prPPS->u4SliceGroupMapType <= 5))
  {
    prSliceHdr->u4SliceGroupChangeCycle = bVDEC_HAL_H264_GetBitStreamFlg(0, u4InstID);
  }
}


VOID vAVC_SLT_Parser(UINT32 u4InstID, UINT32 u4VFifoSa, UINT32 u4PreSa)
{
  UINT32 u4Temp;
  BOOL fgForbidenZeroBits;
  UINT32 u4Bits;
  VDEC_INFO_H264_BS_INIT_PRM_T rH264BSInitPrm;
  VDEC_HAL_H264_DEC_PRM_T *prH264HalPrm; 
  
  prH264HalPrm = VDec_GetH264HalParam(u4InstID);

  do
  {
    _u4AVCCurrPicStartAddr[u4InstID] = u4VDEC_HAL_H264_ReadRdPtr(0, u4InstID, u4VFifoSa, &u4Bits);
    rH264BSInitPrm.u4VFifoSa = u4VFifoSa;
    rH264BSInitPrm.u4VFifoEa = u4VFifoSa + AVC_SLT_V_FIFO_SZ;
    rH264BSInitPrm.u4VLDRdPtr = u4VFifoSa + _u4AVCCurrPicStartAddr[u4InstID];
    rH264BSInitPrm.u4VLDWrPtr = rH264BSInitPrm.u4VFifoEa + 0x400000;

    rH264BSInitPrm.u4PredSa = u4PreSa;
    vH264BSParamConfig(&rH264BSInitPrm,&prH264HalPrm->rBSHALParam);
    i4VDEC_HAL_H264_InitBarrelShifter(0, u4InstID, &prH264HalPrm->rBSHALParam);    
    
    u4Temp = u4VDEC_HAL_H264_GetStartCode_8530(0, u4InstID) & 0xff;
    fgForbidenZeroBits = ((u4Temp >> 7) & 0x01); // bit 31
    if(fgForbidenZeroBits != 0)
    {
      break;
    }
    _u4NalRefIdc[u4InstID] = ((u4Temp >> 5) & 0x3); // bit 30,29
    _ucNalUnitType[u4InstID] = (u4Temp & 0x1f); // bit 28,27,26,25,24

    switch(_ucNalUnitType[u4InstID])
    {
      case IDR_SLICE:
        vAVC_SLT_ParseSliceHeader(u4InstID);
        break;
      case H264_SEI:
        vAVC_SLT_VerifySEI_Rbsp(u4InstID);
        break;
      case H264_SPS:
        vAVC_SLT_VerifySeq_Par_Set_Rbsp(u4InstID);
        break;      
      case H264_PPS:
        vAVC_SLT_VerifyPic_Par_Set_Rbsp(u4InstID);
        break;         
      default:
        break;
    }
  }while(!((_ucNalUnitType[u4InstID] == NON_IDR_SLICE) || (_ucNalUnitType[u4InstID] == IDR_SLICE)));
}

extern char vba_data[];
extern char vba_data_end[];
static UINT32 align(int x, int y)
{
    return (x + y - 1) & ~(y - 1);
}

void getcrop(struct DecCropParams *p_crop)
{
    VDEC_INFO_H264_SPS_T *sps = _tVDecPrm[0].SpecDecPrm.rVDecH264DecPrm.prSPS;
    UINT32 tmp2 = 0;
    if (sps && sps->fgFrameCroppingFlag) {
            UINT32 tmp1[4] = {1, 2, 2, 1}; //width
            UINT32 tmph[4] = {1, 2, 1, 1}; //height
            tmp2 = sps->fgFrameMbsOnlyFlag ? 1 : 2;
            p_crop->crop_left_offset = tmp1[sps->u4ChromaFormatIdc] * sps->u4FrameCropLeftOffset;
            p_crop->crop_out_width = 16 * (sps->u4PicWidthInMbsMinus1+1) -
                                     tmp1[sps->u4ChromaFormatIdc] * (sps->u4FrameCropLeftOffset + sps->u4FrameCropRightOffset);
            p_crop->crop_top_offset = tmph[sps->u4ChromaFormatIdc] * tmp2 * sps->u4FrameCropTopOffset;
            p_crop->crop_out_height = 16 * (sps->u4PicHeightInMapUnitsMinus1+1) -
                                      tmph[sps->u4ChromaFormatIdc] * tmp2 * (sps->u4FrameCropTopOffset +
                                              sps->u4FrameCropBottomOffset);
        } else {
            p_crop->crop_left_offset = 0;
            p_crop->crop_out_width = sps ? (sps->u4PicWidthInMbsMinus1+1) * 16 : 0;
            p_crop->crop_top_offset = 0;
            p_crop->crop_out_height = sps ? (sps->u4PicHeightInMapUnitsMinus1+1) * 16 : 0;
        }

}
void resetHW()
{
    i4VDEC_HAL_Common_Init(0);//set clock
    vVDec_HAL_COMMON_ResetHW(0, VDEC_H264);
    i4VDEC_HAL_Common_Uninit();
}

RSV_MEM_T *mm_reserved = NULL;
void decode(UINT32 u4VFifoStartAddr, UINT32 u4VFifoEndAddr, UINT32 u4AuStart, UINT32 u4AuSize)
{
    UINT32 i,u4VldByte, u4Bits;
    UINT32 u4InstID = 0;
    static UINT32 u4Index = 0;
    BOOL fgVdecAVCSLTResult = TRUE;
    UINT32 u4VFIFOSa = 0;
    UINT32 u4AVCPreSa = 0;
    BYTE *pbAVCVFIFOBuf = NULL;
    BYTE* pbAVCPredSa = NULL;

	VDEC_INFO_H264_BS_INIT_PRM_T rH264VdecBSInfo;
    VDEC_HAL_H264_DEC_PRM_T *prH264HalPrm;
    UINT32 u4ClipTotalFrameCnt = 3;
    UINT32 u4ProcessFrameCnt = 0; 
    UINT32 u4CntTimeChk = 0;
    UINT32 u4CheckCnt = 40000;
    //dump_rsv_mem_info();
    //RSV_MEM_T *mm_reserved = get_rsv_mem_by_name("multimedia");
    UINT32 mm_base = (UINT32)align(mm_reserved->start_addr, 0x1000);
    AVC_SLT_V_FIFO_SZ = u4VFifoEndAddr - u4VFifoStartAddr;
    AVC_SLT_V_FIFO_SZ = align(AVC_SLT_V_FIFO_SZ, 0x1000);
    mm_base += AVC_SLT_V_FIFO_SZ;
    AVC_SLT_DPB_SZ = mm_reserved->size - AVC_SLT_PRED_SZ - AVC_SLT_V_FIFO_SZ;

    AVC_SLT_DPB_BUF_ADDR = ARM1PHY2ARM2UCV(mm_base) + _ptFBufInfo[0][0].u4DramPicArea * (u4Index++ % MAX_OUTPUT);//0x10000// 0xcc000000
    AVC_SLT_PRED_BUF_ADDR = AVC_SLT_DPB_BUF_ADDR + AVC_SLT_DPB_SZ;

    u4VFIFOSa = u4VFifoStartAddr;
#ifdef DEBUG_VDEC
    pbAVCVFIFOBuf = (UINT32)ARM1PHY2ARM2UCV((UINT32) u4VFIFOSa);
    Printf("pattern arm2phyaddr: 0x%x, arm1phyadress:  0x%x, 0x%x 0x%x, 0x%x 0x%x, u4AuStart[0x%x], u4AuSize[0x%x] time %d\n", (UINT32)pbAVCVFIFOBuf, u4VFIFOSa,
        pbAVCVFIFOBuf[0], pbAVCVFIFOBuf[1], pbAVCVFIFOBuf[2], pbAVCVFIFOBuf[3], u4AuStart, u4AuSize, GetBootTime());
#endif
    _pbAVCDPBBuf = (BYTE *)AVC_SLT_DPB_BUF_ADDR;
    _pu4AVCDPBBUF =(UINT32)ARM2UCV2ARM1PHY((UINT32) _pbAVCDPBBuf);
#ifdef DEBUG_VDEC
    Printf("DPBBUF arm2phyaddr: 0x%x, arm1phyadress:  0x%x u4DramPicArea 0x%x\n", (UINT32)_pbAVCDPBBuf, _pu4AVCDPBBUF, _ptFBufInfo[0][0].u4DramPicArea);
#endif
    pbAVCPredSa = (BYTE *)AVC_SLT_PRED_BUF_ADDR;
    u4AVCPreSa =(UINT32)ARM2UCV2ARM1PHY((UINT32) pbAVCPredSa);

    i4VDEC_HAL_Common_Init(0);//set clock

    //Parsing Data
    //VDec HW Init Flow  
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum = 0xff;
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastPicW = 0;
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastPicH = 0;
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prSEI = &_rESI[u4InstID];
    prH264HalPrm = VDec_GetH264HalParam(u4InstID);

    vAVC_SLT_VerifyFlushBufInfo(u4InstID);
    vAVC_SLT_VerifyFlushAllSetData(u4InstID);

    i4VDEC_HAL_H264_InitVDecHW(0);

    rH264VdecBSInfo.u4VFifoSa = (UINT32)u4VFIFOSa;
    rH264VdecBSInfo.u4VFifoEa = (UINT32) u4VFIFOSa + AVC_SLT_V_FIFO_SZ;
    rH264VdecBSInfo.u4VLDRdPtr = (UINT32) u4VFIFOSa + u4AuStart;
    rH264VdecBSInfo.u4VLDWrPtr = rH264VdecBSInfo.u4VFifoEa + 0x400000;
    rH264VdecBSInfo.u4PredSa = (UINT32)u4AVCPreSa;
    vH264BSParamConfig(&rH264VdecBSInfo,&prH264HalPrm->rBSHALParam);///zhi0308
    i4VDEC_HAL_H264_InitBarrelShifter(0, u4InstID, &prH264HalPrm->rBSHALParam);
    
    //parser
    vAVC_SLT_Parser(0, u4VFIFOSa, u4AVCPreSa);//need check

    //Trigger Decode
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.fgIsAllegMvcCfg = 0;
    _tVDecPrm[u4InstID].ucAddrSwapMode = 4;
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucNalRefIdc = _u4NalRefIdc[u4InstID];
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.fgIsFrmPic = fgSLTIsFrmPic(u4InstID);
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.fgIsIDRPic = fgSLTIsIDRPic(u4InstID);
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prCurrFBufInfo = _ptCurrFBufInfo[u4InstID];

    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prCurrFBufInfo->u4YStartAddr = ((UINT32)_pucAVCDecWorkBuf[u4InstID]);
    vH264DecodeParamConfig(&_tVDecPrm[u4InstID], prH264HalPrm);///zhi0308
    i4VDEC_HAL_H264_DecStart(u4InstID, prH264HalPrm);

}


void waitVdecHwReady(VIDINFO *vidInfo)
{
    UINT32 u4CntTimeChk = 0;
    UINT32 u4CheckCnt = 40000;
    UINT32 u4InstID = 0;

    while (u4VDEC_HAL_H264_VDec_ReadFinishFlag(u4InstID) == 0)
    {
        u4CntTimeChk++;
        u4CheckCnt = 0;
        if(u4CntTimeChk > HW_DECODE_TIME_OUT)
            break;
    }

    if (u4CntTimeChk > HW_DECODE_TIME_OUT)
    {
        Printf("[AVC SLT] Decode time out!!\n");
        Printf("AVC SLT Fail \n");
        return FALSE;
    }

    u4CntTimeChk = 0;
    while (fgAVC_SLT_VDecComplete(u4InstID) == FALSE)
    {
        u4CntTimeChk++;
        u4CheckCnt = 0;
        if(u4CntTimeChk > HW_DECODE_TIME_OUT)
            break;
    }

    if (u4CntTimeChk > HW_DECODE_TIME_OUT)
    {
        Printf("[AVC SLT] Decode time out!!\n");
        Printf("AVC SLT Fail \n");
        return FALSE;
    }

    i4VDEC_HAL_Common_Uninit();
    vidInfo->u4VDPDstYPA = _pu4AVCDPBBUF;
    vidInfo->u4VDPDstCPA = _pu4AVCDPBBUF + _ptFBufInfo[0][0].u4DramPicSize;
    vidInfo->width = _tVDecPrm->u4PicW;
    vidInfo->height = _tVDecPrm->u4PicH;

#ifdef DEBUG_VDEC
    Printf("AVC SLT PASS \n");
#endif

}

void noWaitVdecHwReady(VIDINFO *vidInfo)
{
    vidInfo->u4VDPDstYPA = _pu4AVCDPBBUF;
    vidInfo->u4VDPDstCPA = _pu4AVCDPBBUF + _ptFBufInfo[0][0].u4DramPicSize;
    vidInfo->width = _tVDecPrm->u4PicW;
    vidInfo->height = _tVDecPrm->u4PicH;
#ifdef DEBUG_VDEC
    Printf("AVC SLT PASS noWaitVdecHwReady \n");
#endif

}

//Add Code for AVC SLT
BOOL fgVDec_SLT_AVC_Proc(UINT32 u4VFifoStartAddr, UINT32 u4VFifoEndAddr, UINT32 u4AuStart, UINT32 u4AuSize, UINT32 *u4VDPDstYPA, UINT32 *u4VDPDstCPA, UINT32 *width, UINT32 *height)
{
    UINT32 i,u4VldByte, u4Bits;
    UINT32 u4InstID = 0;
    static UINT32 u4Index = 0;
    BOOL fgVdecAVCSLTResult = TRUE;
    UINT32 u4VFIFOSa = 0;
    UINT32 u4AVCPreSa = 0;
    BYTE *pbAVCVFIFOBuf = NULL;
    BYTE* pbAVCPredSa = NULL;

	VDEC_INFO_H264_BS_INIT_PRM_T rH264VdecBSInfo;
    VDEC_HAL_H264_DEC_PRM_T *prH264HalPrm;
    UINT32 u4ClipTotalFrameCnt = 3;
    UINT32 u4ProcessFrameCnt = 0; 
    UINT32 u4CntTimeChk = 0;
    UINT32 u4CheckCnt = 40000;
    //dump_rsv_mem_info();
    RSV_MEM_T *mm_reserved = get_rsv_mem_by_name("multimedia");
    UINT32 mm_base = (UINT32)align(mm_reserved->start_addr, 0x1000);
    AVC_SLT_V_FIFO_SZ = u4VFifoEndAddr - u4VFifoStartAddr;
    AVC_SLT_V_FIFO_SZ = align(AVC_SLT_V_FIFO_SZ, 0x1000);
    mm_base += AVC_SLT_V_FIFO_SZ;
    AVC_SLT_DPB_SZ = mm_reserved->size - AVC_SLT_PRED_SZ - AVC_SLT_V_FIFO_SZ;

    AVC_SLT_DPB_BUF_ADDR = ARM1PHY2ARM2UCV(mm_base) + _ptFBufInfo[0][0].u4DramPicArea * (u4Index++ % 3);//0x10000// 0xcc000000
    AVC_SLT_PRED_BUF_ADDR = AVC_SLT_DPB_BUF_ADDR + AVC_SLT_DPB_SZ;

    u4VFIFOSa = u4VFifoStartAddr;
    #ifdef DEBUG_VDEC
    pbAVCVFIFOBuf = (UINT32)ARM1PHY2ARM2UCV((UINT32) u4VFIFOSa);
    Printf("pattern arm2phyaddr: 0x%x, arm1phyadress:  0x%x, 0x%x 0x%x, 0x%x 0x%x, u4AuStart[0x%x], u4AuSize[0x%x] time %d\n", (UINT32)pbAVCVFIFOBuf, u4VFIFOSa,
        pbAVCVFIFOBuf[0], pbAVCVFIFOBuf[1], pbAVCVFIFOBuf[2], pbAVCVFIFOBuf[3], u4AuStart, u4AuSize, GetBootTime());
    #endif
    _pbAVCDPBBuf = (BYTE *)AVC_SLT_DPB_BUF_ADDR;
    _pu4AVCDPBBUF =(UINT32)ARM2UCV2ARM1PHY((UINT32) _pbAVCDPBBuf);

    #ifdef DEBUG_VDEC
    Printf("DPBBUF arm2phyaddr: 0x%x, arm1phyadress:  0x%x u4DramPicArea 0x%x\n", (UINT32)_pbAVCDPBBuf, _pu4AVCDPBBUF, _ptFBufInfo[0][0].u4DramPicArea);
    #endif
    pbAVCPredSa = (BYTE *)AVC_SLT_PRED_BUF_ADDR;
    u4AVCPreSa =(UINT32)ARM2UCV2ARM1PHY((UINT32) pbAVCPredSa);

    i4VDEC_HAL_Common_Init(0);//set clock

    //Parsing Data
    //VDec HW Init Flow  
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucMaxFBufNum = 0xff;
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastPicW = 0;
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.rLastInfo.u4LastPicH = 0;
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prSEI = &_rESI[u4InstID];
    prH264HalPrm = VDec_GetH264HalParam(u4InstID);

    vAVC_SLT_VerifyFlushBufInfo(u4InstID);
    vAVC_SLT_VerifyFlushAllSetData(u4InstID);

    i4VDEC_HAL_H264_InitVDecHW(0);

    rH264VdecBSInfo.u4VFifoSa = (UINT32)u4VFIFOSa;
    rH264VdecBSInfo.u4VFifoEa = (UINT32) u4VFIFOSa + AVC_SLT_V_FIFO_SZ;
    rH264VdecBSInfo.u4VLDRdPtr = (UINT32) u4VFIFOSa + u4AuStart;
    rH264VdecBSInfo.u4VLDWrPtr = rH264VdecBSInfo.u4VFifoEa + 0x400000;
    rH264VdecBSInfo.u4PredSa = (UINT32)u4AVCPreSa;
    vH264BSParamConfig(&rH264VdecBSInfo,&prH264HalPrm->rBSHALParam);///zhi0308
    i4VDEC_HAL_H264_InitBarrelShifter(0, u4InstID, &prH264HalPrm->rBSHALParam);
    Printf(" AVC_SLT_Parser time :%d\n", GetBootTime());
    
    //parser
    vAVC_SLT_Parser(0, u4VFIFOSa, u4AVCPreSa);//need check
    Printf(" AVC_SLT_Parser time end :%d\n", GetBootTime());

    //Trigger Decode
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.fgIsAllegMvcCfg = 0;
    _tVDecPrm[u4InstID].ucAddrSwapMode = 4;
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.ucNalRefIdc = _u4NalRefIdc[u4InstID];
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.fgIsFrmPic = fgSLTIsFrmPic(u4InstID);
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.fgIsIDRPic = fgSLTIsIDRPic(u4InstID);
    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prCurrFBufInfo = _ptCurrFBufInfo[u4InstID];

    _tVDecPrm[u4InstID].SpecDecPrm.rVDecH264DecPrm.prCurrFBufInfo->u4YStartAddr = ((UINT32)_pucAVCDecWorkBuf[u4InstID]);
    vH264DecodeParamConfig(&_tVDecPrm[u4InstID], prH264HalPrm);///zhi0308
    i4VDEC_HAL_H264_DecStart(u4InstID, prH264HalPrm);
    Printf("[Arm2] dec start time end %d\n", GetBootTime());

    //Verify Result
    //Check Decode End
    u4CntTimeChk = 0;
    while (u4VDEC_HAL_H264_VDec_ReadFinishFlag(u4InstID) == 0)
    {
        u4CntTimeChk++;
        u4CheckCnt = 0;
        if(u4CntTimeChk > HW_DECODE_TIME_OUT)
            break;
    }

    if (u4CntTimeChk > HW_DECODE_TIME_OUT)
    {
        Printf("[AVC SLT] Decode time out!!\n");
        Printf("AVC SLT Fail \n");
        return FALSE;
    }

    u4CntTimeChk = 0;
    while (fgAVC_SLT_VDecComplete(u4InstID) == FALSE)
    {
        u4CntTimeChk++;
        u4CheckCnt = 0;
        if(u4CntTimeChk > HW_DECODE_TIME_OUT)
            break;
    }

    if (u4CntTimeChk > HW_DECODE_TIME_OUT)
    {
        Printf("[AVC SLT] Decode time out!!\n");
        Printf("AVC SLT Fail \n");
        return FALSE;
    }

    //Check Golden Data
    //fgVdecAVCSLTResult = fgH264_SLT_CheckCRCResult(u4InstID, 0);

    i4VDEC_HAL_Common_Uninit();
    *u4VDPDstYPA = _pu4AVCDPBBUF;
    *u4VDPDstCPA = _pu4AVCDPBBUF + _ptFBufInfo[0][0].u4DramPicSize;
    *width = _tVDecPrm->u4PicW;
    *height = _tVDecPrm->u4PicH;

    if(fgVdecAVCSLTResult)
    {
        Printf("[AVC SLT] AVC SLT Pass\n");
        /*for (int i = 0; i <  _tVDecPrm->u4PicH ; i++){
            for (int j = 0; j < _tVDecPrm->u4PicW ; j++) {
                Printf("0x%02x", _pbAVCDPBBuf[i*_tVDecPrm->u4PicW+j]);
            }
            Printf("\n");
        }
         for (int i = 0; i < _tVDecPrm->u4PicH ; i++){
            for (int j = 0; j < _tVDecPrm->u4PicW/2 ; j++) {
                Printf("0x%02x", _pbAVCDPBBuf[_ptFBufInfo[0][0].u4DramPicSize+i* _tVDecPrm->u4PicW/2+j]);
            }
            Printf("\n");
        }*/
        return TRUE;
    }
    else
    {
        Printf("[AVC SLT]AVC SLT Fail \n");
        return FALSE;
    }
}

#endif //AVC_HAL_SUPPORT_SLT

#endif


