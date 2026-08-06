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
#ifndef _VDEC_DRV_H264_INFO_H_
#define _VDEC_DRV_H264_INFO_H_

#include "x_typedef.h"
#include "vdec_usage.h"

#include "vdec_info_h264.h"
#include "vdec_info_common.h"

#include "vdec_hal_if_h264.h"
#include "vdec_fbm.h"


#define NON_IDR_SLICE       0x01
#define IDR_SLICE           0x05
#define H264_SEI            0x06
#define H264_SPS            0x07
#define H264_PPS            0x08
#define H264_END_SEQ        0x0A
#define H264_PREFIX_NAL     0x0E
#define H264_SUB_SPS        0x0F
#define H264_SLICE_EXT      0x14

// Slice_type
#define H264_P_Slice        0
#define H264_B_Slice        1
#define H264_I_Slice        2
#define H264_SP_Slice       3
#define H264_SI_Slice       4
#define H264_P_Slice_ALL    5
#define H264_B_Slice_ALL    6
#define H264_I_Slice_ALL    7
#define H264_SP_Slice_ALL   8
#define H264_SI_Slice_ALL   9

#define NREF_PIC    0
#define SREF_PIC    1
#define LREF_PIC    2

//AVC Profile IDC definitions
#define FREXT_HP_PROFILE                    100     //!< YUV 4:2:0/8 "High"
#define FREXT_Hi10P_PROFILE                 110     //!< YUV 4:2:0/10 "High 10"
#define FREXT_Hi422_PROFILE                 122     //!< YUV 4:2:2/10 "High 4:2:2"
#define FREXT_Hi444_PROFILE                 244     //!< YUV 4:4:4/14 "High 4:4:4"

#define H264_MAX_FB_NUM             45

#define H264_MAX_SPS_NUM        32
#define H264_MAX_PPS_NUM        256
#define H264_MAX_REF_LIST_NUM   6

typedef struct _H264_DRV_INFO_T
{
    UCHAR ucH264DpbOutputFbId;
    UCHAR ucLastH264DpbOutputFbId;
    UCHAR ucPredFbId;
    UINT32 u4PredSa;
    UINT32 u4VPredSa;
    UINT32 u4CurrStartCodeAddr;
    UINT32 u4BitCount;
    UINT32 u4DecErrInfo;
    INT32 i4LatestIPOC;
    INT64 i8BasePTS;
    INT64 i8LatestRealPTS;
    INT32 i4PreFrmPOC;
    INT32 i4LatestRealPOC;
    UINT32 u4LatestSPSId;
    UINT32 u4SPSIDBak;
    UINT32 u4PPSIDBak;
    VDEC_INFO_H264_SPS_T arH264SPS[H264_MAX_SPS_NUM];
    VDEC_INFO_H264_SPS_T arH264SPSBak;
    VDEC_INFO_H264_PPS_T arH264PPS[H264_MAX_PPS_NUM];
    VDEC_INFO_H264_PPS_T arH264PPSBak;
    VDEC_INFO_H264_SLICE_HDR_T rH264SliceHdr;
    VDEC_INFO_H264_SEI_T rH264SEI;
    VDEC_INFO_H264_FBUF_INFO_T arH264FbInfo[H264_MAX_FB_NUM];

    VDEC_INFO_H264_REF_PIC_LIST_T arH264RefPicList[H264_MAX_REF_LIST_NUM];
    VDEC_INFO_H264_P_REF_PRM_T rPRefPicListInfo;
    VDEC_INFO_H264_B_REF_PRM_T rBRefPicListInfo;
    VDEC_INFO_H264_BS_INIT_PRM_T rBsInitPrm;
    VDEC_NORM_INFO_T *prVDecNormInfo;
    VDEC_FBM_INFO_T *prVDecFbmInfo;
    VDEC_PIC_INFO_T *prVDecPicInfo;
    VDEC_INFO_DEC_PRM_T rVDecNormHalPrm;
    void *prH264MvStartAddr;

    //Patch for .mp4 file
    BOOL    fgHeaderDefineByCFA;
    UINT32  u4SPSHeaderCnt;
    UINT32  u4PPSHeaderCnt;

    BOOL    fgFirstAU;
    BOOL    fgNormalFrame;
    BOOL    fgStop;
    BOOL    fgLevelUnSupport;
    BOOL    fgNoMem;
} H264_DRV_INFO_T;

#define fgIsIDRPic(arg) ((arg == IDR_SLICE))
#define fgIsFrmPic(arg) ((arg == FRAME))//slt
//#define fgIsFrmPic(arg) ((_rH264DecPrm[arg].bPicStruct == FRAME))  prVDecH264DecPrm->fgIsFrmPic

#define fgIsISlice(bType) ((bType == H264_I_Slice) || (bType == H264_SI_Slice) || (bType == H264_I_Slice_ALL))
#define fgIsPSlice(bType) ((bType == H264_P_Slice) || (bType == H264_SP_Slice) || (bType == H264_P_Slice_ALL))
#define fgIsBSlice(bType) ((bType == H264_B_Slice) || (bType == H264_B_Slice_ALL))
extern BOOL fgIsNonRefFbuf(VDEC_INFO_H264_FBUF_INFO_T *tFBufInfo);
extern BOOL vH264DecodeParamConfig(VDEC_INFO_DEC_PRM_T *prDecPrm,VDEC_HAL_H264_DEC_PRM_T *prH264HalPrm);
extern VDec_FBUF_INFO_T g_arVDecH264FBuf[H264_MAX_FB_NUM];

#endif

