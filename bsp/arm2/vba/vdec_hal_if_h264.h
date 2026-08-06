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
#ifndef _VDEC_HAL_IF_H264_H_
#define _VDEC_HAL_IF_H264_H_

#include "vdec_info_h264.h"
#include "vdec_info_common.h"
#include "vdec_hal_if_common.h"

/*
For common hal structure
*/

typedef struct _VDEC_HAL_H264_BS_INIT_PRM_T_
{
    UINT32  u4VLDRdPtr;
    UINT32  u4VLDWrPtr;
    UINT32  u4VFifoSa;                 ///< Video Fifo memory start address
    UINT32  u4VFifoEa;                 ///< Video Fifo memory end address
    UINT32  u4PredSa;
}VDEC_HAL_H264_BS_INIT_PRM_T;

typedef struct _VDEC_HAL_H264_P_REF_PRM_T_
{
    UCHAR   ucFBufIdx;
    UINT32  u4FBufInfo;
    UINT32  u4ListIdx;
    UINT32  u4FBufYStartAddr;
    UINT32  u4FBufCAddrOffset;
    UINT32  u4FBufMvStartAddr;
    INT32    i4TFldPOC;
    INT32    i4BFldPOC;
    UINT32  u4TFldPara;
    UINT32  u4BFldPara;
    INT32    i4TFldLongTermPicNum;
    INT32    i4BFldLongTermPicNum;
    INT32    i4LongTermPicNum;
    INT32    i4PicNum;
    INT32    i4TFldPicNum;
    INT32    i4BFldPicNum;
}_VDEC_HAL_H264_P_REF_PRM_T_;


typedef struct _VDEC_HAL_H264_SPS_T_
{
    UINT32  u4Log2MaxFrameNumMinus4;                       // ue(v)
    UINT32  u4PicOrderCntType;
    UINT32  u4Log2MaxPicOrderCntLsbMinus4;              // ue(v)
    UINT32  u4NumRefFrames;                                              // ue(v)
    UINT32  u4PicWidthInMbsMinus1;                              // ue(v)
    UINT32  u4PicHeightInMapUnitsMinus1;                  // ue(v)
    UINT32  u4ChromaFormatIdc;
    BOOL     fgFrameMbsOnlyFlag;                                     // u(1)
    BOOL     fgMbAdaptiveFrameFieldFlag;                      // u(1)
    BOOL     fgDirect8x8InferenceFlag;                             // u(1)
    BOOL     fgDeltaPicOrderAlwaysZeroFlag;                   // u(1)
}VDEC_HAL_H264_SPS_T;

typedef struct _VDEC_HAL_H264_PPS_T_
{
    BOOL     fgEntropyCodingModeFlag;                            // u(1)
    BOOL     fgTransform8x8ModeFlag;                             // u(1)
    BOOL     fgPicOrderPresentFlag;                                   // u(1)
    BOOL     fgDeblockingFilterControlPresentFlag;         // u(1)
    BOOL     fgConstrainedIntraPredFlag;                          // u(1)
    BOOL     fgWeightedPredFlag;                                         // u(1)
    UINT32  u4NumRefIdxL0ActiveMinus1;                    // ue(v)
    UINT32  u4NumRefIdxL1ActiveMinus1;                     // ue(v)
    UINT32  u4WeightedBipredIdc;                                      // u(2)
    INT32    i4PicInitQpMinus26;                                      // se(v)
    INT32    i4ChromaQpIndexOffset;                               // se(v)
    INT32    i4SecondChromaQpIndexOffset;                  // se(v)
} VDEC_HAL_H264_PPS_T;


typedef struct _VDEC_HAL_H264_SLICE_HDR_T_
{
    UINT32  u4FirstMbInSlice;
    UINT32  u4SliceType;
    UINT32  u4NumRefIdxL0ActiveMinus1;
    UINT32  u4NumRefIdxL1ActiveMinus1;
    UINT32  u4DisableDeblockingFilterIdc;
    INT32    i4SliceAlphaC0OffsetDiv2;
    INT32    i4SliceBetaOffsetDiv2;
    INT32    i4SliceQpDelta;
    UINT32   u4CabacInitIdc;
    BOOL     fgFieldPicFlag;
    BOOL     fgBottomFieldFlag;
    BOOL     fgDirectSpatialMvPredFlag;
}VDEC_HAL_H264_SLICE_HDR_T;

typedef struct _VDEC_HAL_INFO_H264_P_REF_PRM_T_
{
    UCHAR   ucFBufIdx;
    UINT32  u4FBufInfo;
    UINT32  u4ListIdx;
    UINT32  u4FBufYStartAddr;
    UINT32  u4FBufCAddrOffset;
    UINT32  u4FBufMvStartAddr;
    INT32    i4TFldPOC;
    INT32    i4BFldPOC;
    UINT32  u4TFldPara;
    UINT32  u4BFldPara;
    INT32    i4TFldLongTermPicNum;
    INT32    i4BFldLongTermPicNum;
    INT32    i4LongTermPicNum;
    INT32    i4PicNum;
    INT32    i4TFldPicNum;
    INT32    i4BFldPicNum;
}VDEC_HAL_INFO_H264_P_REF_PRM_T;

typedef struct _VDEC_HAL_INFO_H264_B_REF_PRM_T_
{
    UCHAR   ucFBufIdx;
    UINT32  u4FBufInfo;
    UINT32  u4ListIdx;
    UINT32  u4FBufYStartAddr;
    UINT32  u4FBufCAddrOffset;
    UINT32  u4FBufMvStartAddr;
    INT32    i4TFldPOC;
    INT32    i4BFldPOC;
    UINT32  u4TFldPara;
    UINT32  u4BFldPara;
    INT32    i4TFldLongTermPicNum;
    INT32    i4BFldLongTermPicNum;
    INT32    i4LongTermPicNum;
    INT32    i4PicNum;
    INT32    i4TFldPicNum;
    INT32    i4BFldPicNum;
    UCHAR  ucFBufIdx1;
    UINT32  u4ListIdx1;
    UINT32  u4FBufYStartAddr1;
    UINT32  u4FBufCAddrOffset1;
    UINT32  u4FBufMvStartAddr1;
    INT32    i4LongTermPicNum1;
    INT32    i4PicNum1;
    INT32    i4TFldPOC1;
    INT32    i4BFldPOC1;
    UINT32  u4TFldPara1;
    UINT32  u4BFldPara1;
}VDEC_HAL_INFO_H264_B_REF_PRM_T;

typedef struct _VDEC_HAL_H264_DEC_PRM_T_
{
    UCHAR   ucPicStruct;
    UCHAR   ucPicType;
    UCHAR   ucDecFBufIdx;
    UCHAR   ucAddrSwapMode;
    UCHAR   ucECLevel;
    BOOL    fgIsFrmPic;
    BOOL    fgIsIDRPic;
    UCHAR   ucNalRefIdc;
    UINT32  u4PicBW;
    UINT32  u4PicW;
    UINT32  u4PicH;
    UINT32  u4YStartAddr;
    UINT32  u4CAddrOffset;
    UINT32  u4MvStartAddr;
    VDEC_HAL_H264_BS_INIT_PRM_T rBSHALParam;
    VDEC_HAL_H264_SPS_T         rSPSHALParam;
    VDEC_HAL_H264_PPS_T         rPPSHALParam;
    VDEC_HAL_H264_SLICE_HDR_T   rSliceHALParam;
    VDEC_HAL_INFO_H264_P_REF_PRM_T  rPRefHalParam;
    VDEC_HAL_INFO_H264_B_REF_PRM_T  rBRefHalParam;

}VDEC_HAL_H264_DEC_PRM_T;


/*! \name Video Decoder HAL H264 Interface
* @{
*/

/// Initialize video decoder hardware
/// \return If return value < 0, it's failed. Please reference hal_vdec_errcode.h.
INT32 i4VDEC_HAL_H264_InitVDecHW(
    UINT32 u4VDecID                                    ///< [IN] Video decoder hardware ID
);


/// Read Barrel Shifter after shifting
/// \return Value of barrel shifter input window after shifting
UINT32 u4VDEC_HAL_H264_ShiftGetBitStream(
    UINT32 u4BSID,                                      ///< [IN] Barrel shifter hardware ID of one video decoder
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    UINT32 u4ShiftBits                                  ///< [IN] Shift bits number
);


/// Read Barrel Shifter before shifting
/// \return Value of barrel shifter input window before shifting
UINT32 u4VDEC_HAL_H264_GetBitStreamShift(
    UINT32 u4BSID,                                      ///< [IN] Barrel shifter hardware ID of one video decoder
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    UINT32 u4ShiftBits                                 ///< [IN] Shift bits number
);


/// Read Barrel Shifter before shifting
/// \return  Most significant (32 - u4ShiftBits) bits of barrel shifter input window before shifting
UINT32 u4VDEC_HAL_H264_GetRealBitStream(
    UINT32 u4BSID,                                      ///< [IN] Barrel shifter hardware ID of one video decoder
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    UINT32 u4ShiftBits                                 ///< [IN] Shift bits number
);


/// Read Barrel Shifter before shifting
/// \return  MSB of barrel shifter input window before shifting
BOOL bVDEC_HAL_H264_GetBitStreamFlg(
    UINT32 u4BSID,                                      ///< [IN] Barrel shifter hardware ID of one video decoder
    UINT32 u4VDecID                                     ///< [IN] Video decoder hardware ID 
);


/// Do UE variable length decoding
/// \return  Input window after UE variable length decoding
UINT32 u4VDEC_HAL_H264_UeCodeNum(
    UINT32 u4BSID,                                      ///< [IN] Barrel shifter hardware ID of one video decoder
    UINT32 u4VDecID                                     ///< [IN] Video decoder hardware ID 
);


/// Do SE variable length decoding
/// \return  Input window after SE variable length decoding
INT32 i4VDEC_HAL_H264_SeCodeNum(
    UINT32 u4BSID,                                      ///< [IN] Barrel shifter hardware ID of one video decoder
    UINT32 u4VDecID                                     ///< [IN] Video decoder hardware ID 
);



/// Get next start code
/// \return Current input window of vld while finding start code 
UINT32 u4VDEC_HAL_H264_GetStartCode_8530(
    UINT32 u4BSID,                                     ///< [IN] Barrel shifter hardware ID of one video decoder
    UINT32 u4VDecID                                    ///< [IN] Video decoder hardware ID 
); 


/// Initialize barrel shifter with byte alignment
/// \return If return value < 0, it's failed. Please reference hal_vdec_errcode.h.
INT32 i4VDEC_HAL_H264_InitBarrelShifter(
    UINT32 u4BSID,                                      ///< [IN] Barrel shifter hardware ID of one video decoder
    UINT32 u4VDecID,                                     ///< [IN] Video decoder hardware ID
    VDEC_HAL_H264_BS_INIT_PRM_T *prH264BSInitPrm
);


/// Read current read pointer
/// \return Current read pointer with byte alignment
UINT32 u4VDEC_HAL_H264_ReadRdPtr(
    UINT32 u4BSID,                                      ///< [IN] Barrel shifter hardware ID of one video decoder
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    UINT32 u4VFIFOSa,
    UINT32 *pu4Bits                                     ///< [OUT] Read pointer with remained bits
);


/// Align read pointer to byte,word or double word
/// \return None
void vVDEC_HAL_H264_AlignRdPtr(
    UINT32 u4BSID,                                      ///< [IN] Barrel shifter hardware ID of one video decoder
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    UINT32 u4AlignType                                  ///< [IN] Align type
);


/// Decode scaling list
/// \return None
void vVDEC_HAL_H264_ScalingList(
    UINT32 u4BSID,                                        ///< [IN] Barrel shifter ID
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    CHAR *pcScalingList,                                  ///< [OUT] Pointer to value of scaling list
    UINT32 u4SizeOfScalingList,                      ///< [IN] Size of scaling list
    BOOL *pfgUseDefaultScalingMatrixFlag        ///< [OUT] Pointer to flag to use default scaling list or not
);


/// Write scaling list to HW
/// \return None
void vVDEC_HAL_H264_WriteScalingList(
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    UINT32 u4Idx,                                          ///< [IN] Index of scaling list
    CHAR *pcSlicePtr                                      ///<[IN] Pointer to list data
);


/// Reference list reordering
/// \return None
void vVDEC_HAL_H264_Reording(
    UINT32 u4VDecID                                    ///< [IN] Video decoder hardware ID
);


/// Decode prediction weighting table
/// \return None
void vVDEC_HAL_H264_PredWeightTable(
    UINT32 u4VDecID                                    ///< [IN] Video decoder hardware ID
);


/// Remove traling bits to byte align
/// \return None
void vVDEC_HAL_H264_TrailingBits(
    UINT32 u4BSID,                                        ///< [IN] Barrel shifter ID
    UINT32 u4VDecID                                    ///< [IN] Video decoder hardware ID
);


/// Check whether there is more rbsp data
/// \return Is morw Rbsp data or not
BOOL bVDEC_HAL_H264_IsMoreRbspData(
    UINT32 u4BSID,                                        ///< [IN] Barrel shifter ID
    UINT32 u4VDecID                                    ///< [IN] Video decoder hardware ID
);


/// Set HW registers to initialize P reference list
/// \return None
void vVDEC_HAL_H264_InitPRefList(
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    BOOL fgPicFrm,
    UINT32 u4MaxFrameNum,                         ///< [IN] Maximium frame number
    UINT32 u4CurrPicNum
);


/// Set HW registers related with P reference list
/// \return None
void vVDEC_HAL_H264_SetPRefPicListReg(
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    VDEC_HAL_INFO_H264_P_REF_PRM_T *prPRefPicListInfo     ///< [IN] Pointer to struct of P reference picutre list information
);


/// Set POC number to HW registers
/// \return None
void vVDEC_HAL_H264_SetPOC(
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    VDEC_INFO_H264_POC_PRM_T *prPOCInfo     ///< [IN] Pointer to struct of POC information
);


/// Set HW registers to initialize B reference list
/// \return None
void vVDEC_HAL_H264_InitBRefList(
    UINT32 u4VDecID                                    ///< [IN] Video decoder hardware ID
);


/// Set HW registers related with B reference list
/// \return B0 list and B1 list are equal or not
BOOL bVDEC_HAL_H264_SetBRefPicListReg(
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    VDEC_HAL_INFO_H264_B_REF_PRM_T *prBRefPicListInfo     ///< [IN] Pointer to struct of B reference picutre list information
);


/// Swap B1 reference list1 
/// \return None
void vVDEC_HAL_H264_B1ListSwap(
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    BOOL fgIsFrmPic                                      ///< [IN] Flag of frame picture or not
);


/// Set SPS data to HW
/// \return None
void vVDEC_HAL_H264_SetSPSAVLD(
    UINT32 u4VDecID,                                   ///< [IN] Video decoder hardware ID
    VDEC_HAL_H264_SPS_T *prSPS           ///< [IN] Pointer to struct of sequence parameter set
);


/// Set PPS data to HW
/// \return None
void vVDEC_HAL_H264_SetPPSAVLD(
    UINT32 u4VDecID,                                   ///< [IN] Video decoder hardware ID
    BOOL fgUserScalingMatrixPresentFlag,
    BOOL *pfgUserScalingListPresentFlag,
    VDEC_HAL_H264_PPS_T *prPPS           ///< [IN] Pointer to struct of picutre parameter set
);


/// Set part of slice header data to HW
/// \return None
void vVDEC_HAL_H264_SetSHDRAVLD1(
    UINT32 u4VDecID,                                              ///< [IN] Video decoder hardware ID
    VDEC_HAL_H264_SLICE_HDR_T *prSliceHdr        ///< [IN] Pointer to struct of picutre parameter set
);


/// Set video decoder hardware registers to decode
/// \return If return value < 0, it's failed. Please reference hal_vdec_errcode.h.
INT32 i4VDEC_HAL_H264_DecStart(
    UINT32 u4VDecID,
    VDEC_HAL_H264_DEC_PRM_T *prHalH264DecPrm              ///< [IN] Pointer to H264 decode Information
);


/// Read current decoded mbx and mby
/// \return None
void vVDEC_HAL_H264_GetMbxMby(
    UINT32 u4VDecID,                                    ///< [IN] Video decoder hardware ID
    UINT32 *pu4Mbx,                                      ///< [OUT] Pointer to current decoded macroblock in x axis
    UINT32 *pu4Mby                                       ///< [OUT] Pointer to current decoded macroblock in y axis
);



VDEC_HAL_H264_DEC_PRM_T *VDec_GetH264HalParam(UCHAR ucEsId);
//
/*! @} */


#endif //#ifndef _HAL_VDEC_H264_IF_H_

