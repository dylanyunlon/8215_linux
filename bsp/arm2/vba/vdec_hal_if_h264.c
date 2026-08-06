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
#include "vdec_hw_common.h"
#include "vdec_hal_if_h264.h"
#include "vdec_hw_h264.h"
//#include "vdec_hal_errcode.h"
#include "x_hal_ic.h"
#include "x_printf.h"

const CHAR ZZ_SCAN[16]  =
{  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

const CHAR ZZ_SCAN8[64] =
{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

static VDEC_HAL_H264_DEC_PRM_T _arH264HalDecParam;

#ifdef MPV_DUMP_H264_DEC_REG
void VDec_DumpH264Reg(UCHAR ucMpvId);
#endif

// **************************************************************************
// Function : INT32 i4VDEC_HAL_H264_InitVDecHW(UINT32 u4Handle);
// Description :Initialize video decoder hardware only for H264
// Parameter : u4VDecID : video decoder hardware ID
//                  prH264VDecInitPrm : pointer to VFIFO info struct
// Return      : =0: success.
//                  <0: fail.
// **************************************************************************
INT32 i4VDEC_HAL_H264_InitVDecHW(UINT32 u4VDecID)
{

    vVDec_HAL_COMMON_ResetHW(u4VDecID, VDEC_H264);

    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_ERR_MASK, (~(AVLD_MB_END_CHK | AVLD_4BLOCKS_SKIP_CHK)));

    return  HAL_HANDLE_OK;
}


// **************************************************************************
// Function : UINT32 u4VDEC_HAL_H264_ShiftGetBitStream(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits);
// Description :Read barrel shifter after shifting
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 u4ShiftBits : shift bits number
// Return      : Value of barrel shifter input window after shifting
// **************************************************************************
UINT32 u4VDEC_HAL_H264_ShiftGetBitStream(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits)
{
    UINT32 u4RegVal;

    u4RegVal = u4VDecAVCVLDGetBitS(u4BSID, u4VDecID, u4ShiftBits);

    return (u4RegVal);
}


// **************************************************************************
// Function : UINT32 u4VDEC_HAL_H264_GetBitStreamShift(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits);
// Description :Read Barrel Shifter before shifting
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 u4ShiftBits : shift bits number
// Return      : Value of barrel shifter input window before shifting
// **************************************************************************
UINT32 u4VDEC_HAL_H264_GetBitStreamShift(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits)
{
    UINT32 u4RegVal0;

    u4RegVal0 = u4VDecAVCVLDGetBitS(u4BSID, u4VDecID, 0);
    u4VDecAVCVLDGetBitS(u4BSID, u4VDecID, u4ShiftBits);

    return (u4RegVal0);
}


// **************************************************************************
// Function : UINT32 u4VDEC_HAL_H264_GetRealBitStream(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits);
// Description :Read Barrel Shifter before shifting
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 u4ShiftBits : shift bits number
// Return      : Most significant (32 - u4ShiftBits) bits of barrel shifter input window before shifting
// **************************************************************************
UINT32 u4VDEC_HAL_H264_GetRealBitStream(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits)
{
    UINT32 u4RegVal0;

    u4RegVal0 = u4VDecAVCVLDGetBitS(u4BSID, u4VDecID, 0);
    u4VDecAVCVLDGetBitS(u4BSID, u4VDecID, u4ShiftBits);

    return (u4RegVal0 >> (32-u4ShiftBits));
}


// **************************************************************************
// Function : UINT32 bVDEC_HAL_H264_GetBitStreamFlg(UINT32 u4BSID, UINT32 u4VDecID);
// Description :Read Barrel Shifter before shifting
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : MSB of barrel shifter input window before shifting
// **************************************************************************
BOOL bVDEC_HAL_H264_GetBitStreamFlg(UINT32 u4BSID, UINT32 u4VDecID)
{
    UINT32 u4RegVal;

    u4RegVal = u4VDEC_HAL_H264_GetBitStreamShift(u4BSID, u4VDecID, 1);
    return ((u4RegVal >> 31));
}


// **************************************************************************
// Function : UINT32 u4VDEC_HAL_H264_UeCodeNum(UINT32 u4BSID, UINT32 u4VDecID);
// Description :Do UE variable length decoding
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : Input window after UE variable length decoding
// **************************************************************************
UINT32 u4VDEC_HAL_H264_UeCodeNum(UINT32 u4BSID, UINT32 u4VDecID)
{
    if (u4BSID == 0)
    {
        return (u4VDecReadAVCVLD(u4VDecID, RO_AVLD_UE));
    }
    else
    {
        u4VDecReadAVCVLD(u4VDecID, RO_VLD_AV_2ND_BARL);
        return (u4VDecReadAVCVLD(u4VDecID,RO_VLD_AV_2ND_UE));
    }
}


// **************************************************************************
// Function : INT32 i4VDEC_HAL_H264_SeCodeNum(UINT32 u4BSID, UINT32 u4VDecID);
// Description :Do SE variable length decoding
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : Input window after SE variable length decoding
// **************************************************************************
INT32 i4VDEC_HAL_H264_SeCodeNum(UINT32 u4BSID, UINT32 u4VDecID)
{
    if (u4BSID == 0)
    {
        return ((INT32)u4VDecReadAVCVLD(u4VDecID, RO_AVLD_SE));
    }
    else
    {
        u4VDecReadAVCVLD(u4VDecID, RO_VLD_AV_2ND_BARL);
        return ((INT32)u4VDecReadAVCVLD(u4VDecID, RO_VLD_AV_2ND_SE));
    }
}


// *********************************************************************
// Function    : UINT32 u4VDEC_HAL_H264_GetStartCode(UINT32 u4BSID, UINT32 u4VDecID)
// Description : Get next start code
// Parameter   : u4BSID : Barrel shifter ID
//                   u4VDecID : VLD ID
// Return      : None
// *********************************************************************
UINT32 u4VDEC_HAL_H264_GetStartCode_8530 (UINT32 u4BSID, UINT32 u4VDecID)
{
    UINT32 u4Temp = 0;
    BOOL fgVLDRem03;
    UINT32 u4RetryNum = 0x100000;
    UINT32 i;
#if 0 //Xuebin Fix Packet Lost
    VDEC_ES_INFO_T *prVDecEsInfo;
    prVDecEsInfo = VDec_GetEsInfo(0);
#endif

    if (u4BSID == 0)
    {
       u4Temp = u4VDEC_HAL_H264_ShiftGetBitStream(u4BSID, u4VDecID, 0);
       //Printf("u4Temp 0x%x\n", u4Temp);
       if (u4BSID == 0)
       {
           fgVLDRem03 = u4VDecReadAVCVLD(u4VDecID, RW_VLD_AV_CTRL) >> 31;
       }
       else
       {
           fgVLDRem03 = (u4VDecReadAVCVLD(u4VDecID, RW_VLD_AV_2ND_CTRL) & 0x40);
        }

       if (((u4Temp >> 8) != START_CODE) || fgVLDRem03)
       {
       vVDecWriteAVCVLD(u4VDecID,  RW_AVLD_FSSR, FW_SEARCH_START_CODE);

       for (i=0; i < u4RetryNum; i++)
       {
          if ((u4VDecReadAVCVLD(u4VDecID,  RW_AVLD_FSSR) & 0x1) == 0)
          {
               break;
          }
       }

        fgVLDRem03 = (u4VDecReadAVCVLD(u4VDecID,  RW_AVLD_RM03R) >> 11) & 0x1;
        if (i == u4RetryNum && !fgVLDRem03)
       {
           Printf("Can not find AVC start code\n");
               // prVDecEsInfo->fgNoSc = TRUE;
                return u4Temp;
        }
     }
     }
     else
     {
         //VDEC_ASSERT(0);
     }

     u4Temp = u4VDEC_HAL_H264_GetBitStreamShift(u4BSID, u4VDecID, 32);
     //Printf("return u4Temp 0x%x\n", u4Temp);
    return u4Temp;
}


// **************************************************************************
// Function : INT32 i4VDEC_HAL_H264_InitBarrelShifter(UINT32 u4BSID, UINT32 u4VDecID, VDEC_INFO_H264_BS_INIT_PRM_T *prH264BSInitPrm);
// Description :Initialize barrel shifter with byte alignment
// Parameter :u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 prH264BSInitPrm : pointer to h264 initialize barrel shifter information struct
// Return      : =0: success.
//                  <0: fail.
// **************************************************************************
INT32 i4VDEC_HAL_H264_InitBarrelShifter(UINT32 u4BSID, UINT32 u4VDecID, VDEC_HAL_H264_BS_INIT_PRM_T *prH264BSInitPrm)
{
    BOOL fgInitBSResult;

    if (u4BSID == 0)
    {
        fgInitBSResult = fgInitH264BarrelShift1(u4VDecID, prH264BSInitPrm);
    }
    else
    {
        fgInitBSResult = fgInitH264BarrelShift2(u4VDecID, prH264BSInitPrm);
    }

    if (fgInitBSResult)
    {
        return HAL_HANDLE_OK;
    }
    else
    {
        Printf("[AVC SLT] i4VDEC_HAL_H264_InitBarrelShifter FAIL\n");
        return INIT_BARRELSHIFTER_FAIL;
    }
}


// **************************************************************************
// Function : UINT32 u4VDEC_HAL_H264_ReadRdPtr(UINT32 u4BSID, UINT32 u4VDecID, UINT32 *pu4Bits);
// Description :Read current read pointer
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 pu4Bits : read pointer value with remained bits
// Return      : Read pointer value with byte alignment
// **************************************************************************
UINT32 u4VDEC_HAL_H264_ReadRdPtr(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4VFIFOSa, UINT32 *pu4Bits)
{
    return u4VDecReadH264VldRPtr(u4BSID, u4VDecID, pu4Bits, (u4VFIFOSa));
}


// **************************************************************************
// Function : void v4VDEC_HAL_H264_AlignRdPtr(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4AlignType);
// Description :Align read pointer to byte,word or double word
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 u4AlignType : read pointer align type
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_AlignRdPtr(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4AlignType)
{
    return;
}


// **************************************************************************
// Function : void vVDEC_HAL_H264_ScalingList(UINT32 u4BSID, UINT32 u4VDecID, CHAR *pcScalingList, UINT32 u4SizeOfScalingList, BOOL *pfgUseDefaultScalingMatrixFlag);
// Description :Decode scaling list
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 u4SizeOfScalingList : size of scaling list
//                 pcScalingList : pointer to value of scaling list
//                 pfgUseDefaultScalingMatrixFlag : pointer to  flag to use default scaling list or not
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_ScalingList(UINT32 u4BSID, UINT32 u4VDecID, CHAR *pcScalingList, UINT32 u4SizeOfScalingList, BOOL *pfgUseDefaultScalingMatrixFlag)
{
    UINT32 i;
    UINT32 u4Scanj;
    INT32 i4LastScale;
    INT32 i4NextScale;
    INT32 i4DeltaScale;

    i4LastScale = 8;
    i4NextScale = 8;

    for (i=0; i<u4SizeOfScalingList; i++)
    {
        u4Scanj = (u4SizeOfScalingList==16) ? ZZ_SCAN[i]:ZZ_SCAN8[i];

        if (i4NextScale != 0)
        {
            i4DeltaScale = i4VDEC_HAL_H264_SeCodeNum(u4BSID, u4VDecID);
            i4NextScale = (i4LastScale + i4DeltaScale + 256) % 256;
            *pfgUseDefaultScalingMatrixFlag = ((u4Scanj == 0) && (i4NextScale == 0)) ? TRUE : FALSE;
        }
        pcScalingList[u4Scanj] = (i4NextScale == 0)? i4LastScale : i4NextScale;
        i4LastScale = pcScalingList[u4Scanj];
    }
}

// **************************************************************************
// Function : void vVDEC_HAL_H264_WriteScalingList(UINT32 u4VDecID, UINT32 u4Idx, CHAR *pcSlicePtr);
// Description :Write scaling list to HW
// Parameter : u4VDecID : video decoder hardware ID
//                 u4Idx : scaling list index
//                 pcSlicePtr : pointer to scaling list matrix
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_WriteScalingList(UINT32 u4VDecID, UINT32 u4Idx, CHAR *pcSlicePtr)
{
    INT32 i;
    UINT32 u4Temp;

    if (u4Idx < 6)
    {
        u4Idx = (u4Idx << 4);
        for (i=0; i<4; i++)
        {
            // add 16 for every list
            u4Temp = (((UCHAR)pcSlicePtr[i<<2]) << 24) + (((UCHAR)pcSlicePtr[(i<<2) + 1]) << 16) +(((UCHAR)pcSlicePtr[(i<<2) + 2]) << 8) + (((UCHAR)pcSlicePtr[(i<<2) + 3]));
            vVDecWriteVLD(u4VDecID, RW_VLD_SCL_ADDR, 0x200 + u4Idx + (i << 2));
            vVDecWriteVLD(u4VDecID, RW_VLD_SCL_DATA, u4Temp);
            vVDecWriteVLD(u4VDecID, RW_VLD_SCL_ADDR, 0x100 + u4Idx + (i << 2));
        }
    }
    else
    {
        u4Idx = (u4Idx == 6) ? (u4Idx << 4) : ((u4Idx + 3) << 4); // 6=>16*6   7=>16*6+64(equal to 16*7+48)
        for (i=0; i<16; i++)
        {
            // add 64 for every list
            u4Temp = (((UCHAR)pcSlicePtr[i<<2]) << 24) + (((UCHAR)pcSlicePtr[(i<<2) + 1]) << 16) +(((UCHAR)pcSlicePtr[(i<<2) + 2]) << 8) + (((UCHAR)pcSlicePtr[(i<<2) + 3]));
            vVDecWriteVLD(u4VDecID, RW_VLD_SCL_ADDR, 0x200 + u4Idx + (i << 2));
            vVDecWriteVLD(u4VDecID, RW_VLD_SCL_DATA, u4Temp);
            vVDecWriteVLD(u4VDecID, RW_VLD_SCL_ADDR, 0x100 + u4Idx + (i << 2));
        }
    }
}


// **************************************************************************
// Function : void vVDEC_HAL_H264_Reording(UINT32 u4VDecID);
// Description :Reference list reordering
// Parameter : u4VDecID : video decoder hardware ID
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_Reording(UINT32 u4VDecID)
{
    UINT32 u4Cnt;

    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_RPL_REORD, 1);
    u4Cnt = 0;
    while (1)
    {
        if (u4Cnt == 100)
        {
            if (u4VDecReadAVCVLD(u4VDecID, RW_AVLD_RPL_REORD))
            {
                break;
            }
            else
            {
                u4Cnt = 0;
            }
        }
        else
        {
            u4Cnt ++;
        }
    }
}

// **************************************************************************
// Function : void vVDEC_HAL_H264_PredWeightTable(UINT32 u4VDecID);
// Description :Decode prediction weighting table
// Parameter : u4VDecID : video decoder hardware ID
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_PredWeightTable(UINT32 u4VDecID)
{
    UINT32 u4Cnt;

    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_WEIGHT_PRED_TBL, 1);
    u4Cnt = 0;
    while (1)
    {
        if (u4Cnt == 100)
        {
            if (u4VDecReadAVCVLD(u4VDecID, RW_AVLD_WEIGHT_PRED_TBL))
            {
                break;
            }
            else
            {
                u4Cnt = 0;
            }
        }
        else
        {
            u4Cnt ++;
        }
    }

}


// **************************************************************************
// Function : void vVDEC_HAL_H264_TrailingBits(UINT32 u4BSID, UINT32 u4VDecID);
// Description :Remove traling bits to byte align
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_TrailingBits(UINT32 u4BSID, UINT32 u4VDecID)
{
    UINT32 u4Temp;

    u4Temp = 8 - (u4VDecAVCVLDShiftBits(u4BSID, u4VDecID) % 8);
    // at list trailing bit
    if (u4Temp < 8)
    {
        u4Temp = u4VDecAVCVLDGetBitS(u4BSID, u4VDecID, u4Temp);
    }
}


// **************************************************************************
// Function : BOOL bVDEC_HAL_H264_IsMoreRbspData(UINT32 u4BSID, UINT32 u4VDecID);
// Description :Check whether there is more rbsp data
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : Is morw Rbsp data or not
// **************************************************************************
BOOL bVDEC_HAL_H264_IsMoreRbspData(UINT32 u4BSID, UINT32 u4VDecID)
{
    UINT32 u4RemainedBits;
    UINT32 u4Temp;
    INT32 i;

    u4RemainedBits = (u4VDecAVCVLDShiftBits(u4BSID, u4VDecID) % 8); //0~7
    u4Temp = 0xffffffff;
    for (i=0; i<=u4RemainedBits; i++)
    {
        u4Temp &= (~(1<<i));
    }

    if ((u4VDecAVCVLDGetBitS(u4BSID, u4VDecID, 0) & u4Temp) == (0x80000000))
    {
        // no more
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


// **************************************************************************
// Function : void vVDEC_HAL_H264_InitPRefList(UINT32 u4VDecID, BOOL fgPicFrm, UINT32 u4MaxFrameNum, UINT32 u4CurrPicNum);
// Description :Set HW registers to initialize P reference list
// Parameter : u4VDecID : video decoder hardware ID
//                 fgPicFrm : flag of frame picture or not
//                 u4MaxFrameNum : maximium frame number
//                 u4CurrPicNum : current pic number
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_InitPRefList(UINT32 u4VDecID, BOOL fgPicFrm, UINT32 u4MaxFrameNum, UINT32 u4CurrPicNum)
{
    int i;

    vVDecWriteMC(u4VDecID, RW_AMC_P_LIST0_FLD, 0);
    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_RESET_PIC_NUM, RESET_PIC_NUM);

    for (i = 0; i < 10000; i++)
    {
         int a = 0;
         if ( (u4VDecReadAVCVLD(u4VDecID, RW_AVLD_RESET_PIC_NUM) & 0x1) == 0)
         {
            break;
         }

         for (i = 0; i < 1000; i++)
         {
            a += 100;
         }
    }

    if (i == 10000)
    {
        Printf("[VDEC][8550_AVC]: RW_AVLD_RESET_PIC_NUM: Pooling FAIL!!!\n");
    }
    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_MAX_PIC_NUM, fgPicFrm ? u4MaxFrameNum : (u4MaxFrameNum << 1));
    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_CUR_PIC_NUM, u4CurrPicNum);
}


// **************************************************************************
// Function : void vVDEC_HAL_H264_SetPRefPicListReg(UINT32 u4VDecID, VDEC_INFO_H264_P_REF_PRM_T *prPRefPicListInfo);
// Description :Set HW registers related with P reference list
// Parameter : u4VDecID : video decoder hardware ID
//                 prPRefPicListInfo : pointer to information of p reference list
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_SetPRefPicListReg(UINT32 u4VDecID, VDEC_HAL_INFO_H264_P_REF_PRM_T *prPRefPicListInfo)
{
    UCHAR ucFBufIdx;
    UCHAR ucFld;
    //UCHAR bRefPicIdx;
    UCHAR ucRegIdx;
    UINT32 u4Temp;
    BOOL fgLRefPic;
    UINT32 u4Param = 0;

    ucFld = (prPRefPicListInfo->u4FBufInfo & 0xff);
    //bRefPicIdx = ((prPRefPicListInfo->u4FBufInfo >> 8) & 0xff);
    ucRegIdx = ((prPRefPicListInfo->u4FBufInfo >> 16) & 0xff);

    ucFBufIdx = prPRefPicListInfo->ucFBufIdx;
    fgLRefPic = (prPRefPicListInfo->u4ListIdx > 3)? TRUE: FALSE;

    if (ucFld == FRAME)
    {
        //bRegIdx = bRefPicIdx;
        vVDecWriteMC(u4VDecID, RW_MC_P_LIST0 + (ucRegIdx <<2), ((UINT32)  prPRefPicListInfo->u4FBufYStartAddr));
        u4Param = (fgLRefPic << 19) + (fgLRefPic? (prPRefPicListInfo->i4LongTermPicNum & 0x7ffff) : (prPRefPicListInfo->i4PicNum & 0x7ffff));
        vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_P0_RPL + (ucRegIdx<<2), u4Param);

        u4Temp = (((prPRefPicListInfo->i4TFldPOC <= prPRefPicListInfo->i4BFldPOC) && (prPRefPicListInfo->i4TFldPOC != 0x7fffffff))? 1: 0) << 21;
        u4Temp |= (fgLRefPic << 20);
        u4Temp |= ((ucFBufIdx)<<1<<22) + prPRefPicListInfo->u4TFldPara;
        vVDecWriteAVCMV(u4VDecID, RW_AMV_P_REF_PARA + (ucRegIdx<<3), u4Temp);

        u4Temp = (((prPRefPicListInfo->i4TFldPOC <= prPRefPicListInfo->i4BFldPOC) && (prPRefPicListInfo->i4TFldPOC != 0x7fffffff))? 1: 0) << 21;
        u4Temp |= (fgLRefPic << 20);
        u4Temp |= ((((ucFBufIdx)<<1) + 1)<<22) + prPRefPicListInfo->u4BFldPara;
        vVDecWriteAVCMV(u4VDecID, RW_AMV_P_REF_PARA + (ucRegIdx<<3) + 4, u4Temp);
    }
    else
    {
        if (ucFld == TOP_FIELD)
        {
            vVDecWriteMC(u4VDecID, RW_MC_P_LIST0 + (ucRegIdx<<2), ((UINT32) prPRefPicListInfo->u4FBufYStartAddr));
            u4Param = (fgLRefPic << 19) + (fgLRefPic? (prPRefPicListInfo->i4TFldLongTermPicNum & 0x7ffff) : (prPRefPicListInfo->i4TFldPicNum & 0x7ffff));
            vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_P0_RPL + (ucRegIdx<<2), u4Param);

            //u4Temp = (((_ptFBufInfo[ucFBufIdx].i4TFldPOC <= _ptFBufInfo[ucFBufIdx].i4BFldPOC) && (_ptFBufInfo[ucFBufIdx].i4TFldPOC != 0x7fffffff))? 1: 0) << 21;
            u4Temp = (fgLRefPic << 20);
            u4Temp |= ((ucFBufIdx)<<1<<22) + prPRefPicListInfo->u4TFldPara;
            vVDecWriteAVCMV(u4VDecID, RW_AMV_P_REF_PARA + (ucRegIdx<<2), u4Temp);
        }
        else
        {
            vVDecWriteMC(u4VDecID, RW_MC_P_LIST0 + (ucRegIdx<<2), ((UINT32) prPRefPicListInfo->u4FBufYStartAddr));
            //vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_P0_RPL + (ucRegIdx<<2), (fgLRefPic << 19) + (fgLRefPic? (prPRefPicListInfo->i4BFldLongTermPicNum & 0x7ffff) : (prPRefPicListInfo->i4BFldPicNum & 0x7ffff)));
            u4Param = (fgLRefPic << 19) + (fgLRefPic? (prPRefPicListInfo->i4BFldLongTermPicNum & 0x7ffff) : (prPRefPicListInfo->i4BFldPicNum & 0x7ffff));
            vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_P0_RPL + (ucRegIdx<<2), u4Param);
            //UTIL_Printf("SetPRefPicList4: [0x%X] = 0x%X\n", RW_AVLD_REORD_P0_RPL + (ucRegIdx<<2),  u4Param);

            //u4Temp = (((_ptFBufInfo[ucFBufIdx].i4TFldPOC <= _ptFBufInfo[ucFBufIdx].i4BFldPOC) && (_ptFBufInfo[ucFBufIdx].i4TFldPOC != 0x7fffffff))? 1: 0) << 21;
            u4Temp = (fgLRefPic << 20);
            u4Temp |= (((ucFBufIdx<<1) + 1)<<22) + prPRefPicListInfo->u4BFldPara;
            vVDecWriteAVCMV(u4VDecID, RW_AMV_P_REF_PARA + (ucRegIdx<<2), u4Temp);

            vVDecWriteMC(u4VDecID, RW_AMC_P_LIST0_FLD, u4VDecReadMC(u4VDecID, RW_AMC_P_LIST0_FLD) | (0x1 << ucRegIdx));
        }
    }
}


// **************************************************************************
// Function : void vVDEC_HAL_H264_SetPOC(UINT32 u4VDecID, VDEC_INFO_H264_POC_PRM_T *prPOCInfo);
// Description :Set POC number to HW registers
// Parameter : u4VDecID : video decoder hardware ID
//                 prPOCInfo : pointer to information of current POC
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_SetPOC(UINT32 u4VDecID, VDEC_INFO_H264_POC_PRM_T *prPOCInfo)
{
    INT32 i4CurrPOC;

    if (prPOCInfo->fgIsFrmPic)
    {
        i4CurrPOC = prPOCInfo->i4POC;
        vVDecWriteAVCMV(u4VDecID, RW_AMV_CURR_TFLD_POC, prPOCInfo->i4TFldPOC & 0x3ffff);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_CURR_BFLD_POC, prPOCInfo->i4BFldPOC & 0x3ffff);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_CURR_POC, prPOCInfo->i4POC & 0x3ffff);
    }
    else
    {
        i4CurrPOC = (prPOCInfo->ucPicStruct == TOP_FIELD)? prPOCInfo->i4TFldPOC : prPOCInfo->i4BFldPOC;
        vVDecWriteAVCMV(u4VDecID, RW_AMV_CURR_POC, i4CurrPOC & 0x3ffff);
    }
}


// **************************************************************************
// Function : void vVDEC_HAL_H264_InitBRefList(UINT32 u4VDecID);
// Description :Set HW registers to initialize B reference list
// Parameter : u4VDecID : video decoder hardware ID
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_InitBRefList(UINT32 u4VDecID)
{
    vVDecWriteMC(u4VDecID, RW_AMC_B_LIST0_FLD, 0);
    vVDecWriteMC(u4VDecID, RW_AMC_B_LIST1_FLD, 0);
}


// **************************************************************************
// Function : BOOL bVDEC_HAL_H264_SetBRefPicListReg(UINT32 u4VDecID, VDEC_INFO_H264_B_REF_PRM_T *prBRefPicListInfo);
// Description :Set HW registers related with B reference list
// Parameter : u4VDecID : video decoder hardware ID
//                 prBRefPicListInfo : pointer to information of b reference list
// Return      : None
// **************************************************************************
BOOL bVDEC_HAL_H264_SetBRefPicListReg(UINT32 u4VDecID, VDEC_HAL_INFO_H264_B_REF_PRM_T *prBRefPicListInfo)
{
    UCHAR ucFBufIdx;
    UCHAR ucFld;
    //UCHAR bRefPicIdx;
    UCHAR ucRegIdx;
    UINT32 u4Temp;
    BOOL fgLRefPic;
    UINT32 u4FieldDistance = 4;

    ucFld = (prBRefPicListInfo->u4FBufInfo & 0xff);
    //bRefPicIdx = ((prBRefPicListInfo->u4FBufInfo >> 8) & 0xff);
    ucRegIdx = ((prBRefPicListInfo->u4FBufInfo >> 16) & 0xff);

    ucFBufIdx = prBRefPicListInfo->ucFBufIdx;
    fgLRefPic = (prBRefPicListInfo->u4ListIdx > 3)? TRUE: FALSE;

    if (ucFld == FRAME)
    {
        // B_0
        vVDecWriteMC(u4VDecID, RW_MC_B_LIST0 + (ucRegIdx<<2), ((UINT32) prBRefPicListInfo->u4FBufYStartAddr));
        vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_B0_RPL + (ucRegIdx<<2), (fgLRefPic << 19) + (fgLRefPic? (prBRefPicListInfo->i4LongTermPicNum & 0x7ffff) : (prBRefPicListInfo->i4PicNum & 0x7ffff)));

        u4Temp = (((prBRefPicListInfo->i4TFldPOC <= prBRefPicListInfo->i4BFldPOC) && (prBRefPicListInfo->i4TFldPOC != 0x7fffffff))? 1: 0) << 21;
        u4Temp |= (fgLRefPic << 20);
        u4Temp += ((ucFBufIdx)<<1<<22) + prBRefPicListInfo->u4TFldPara + (prBRefPicListInfo->i4TFldPOC & 0x3ffff);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B0_REF_PARA + (ucRegIdx<<3), u4Temp);

        u4Temp = (((prBRefPicListInfo->i4TFldPOC <= prBRefPicListInfo->i4BFldPOC) && (prBRefPicListInfo->i4TFldPOC != 0x7fffffff))? 1: 0) << 21;
        u4Temp |= (fgLRefPic << 20);
        u4Temp += ((((ucFBufIdx)<<1) + 1)<<22) + prBRefPicListInfo->u4BFldPara + (prBRefPicListInfo->i4BFldPOC & 0x3ffff);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B0_REF_PARA + (ucRegIdx<<3) + 4, u4Temp);


        // B_1
        ucFBufIdx = prBRefPicListInfo->ucFBufIdx1;
        vVDecWriteMC(u4VDecID, RW_MC_B_LIST1 + (ucRegIdx<<2), ((UINT32) prBRefPicListInfo->u4FBufYStartAddr1));
        vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_B1_RPL + (ucRegIdx<<2), (fgLRefPic << 19) + (fgLRefPic? (prBRefPicListInfo->i4LongTermPicNum1 & 0x7ffff) : (prBRefPicListInfo->i4PicNum1 & 0x7ffff)));

        u4Temp = (((prBRefPicListInfo->i4TFldPOC1 <= prBRefPicListInfo->i4BFldPOC1) && (prBRefPicListInfo->i4TFldPOC1 != 0x7fffffff))? 1: 0) << 21;
        u4Temp |= (fgLRefPic << 20);
        u4Temp += ((ucFBufIdx)<<1<<22) + prBRefPicListInfo->u4TFldPara1 + (prBRefPicListInfo->i4TFldPOC1 & 0x3ffff);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + (ucRegIdx<<3), u4Temp);

        u4Temp = (((prBRefPicListInfo->i4TFldPOC1 <= prBRefPicListInfo->i4BFldPOC1) && (prBRefPicListInfo->i4TFldPOC1 != 0x7fffffff))? 1: 0) << 21;
        u4Temp |= (fgLRefPic << 20);
        u4Temp += ((((ucFBufIdx)<<1) + 1)<<22) + prBRefPicListInfo->u4BFldPara1 + (prBRefPicListInfo->i4BFldPOC1 & 0x3ffff);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + (ucRegIdx<<3) + 4, u4Temp);

        u4Temp = ((UINT32) prBRefPicListInfo->u4FBufMvStartAddr1);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + (ucRegIdx<<3), u4Temp >> 4);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + (ucRegIdx<<3) + 4, (u4Temp >> 4) + u4FieldDistance);
    }
    else
    {
      // B_0
      if ((prBRefPicListInfo->u4ListIdx == 0) || (prBRefPicListInfo->u4ListIdx == 1) || (prBRefPicListInfo->u4ListIdx == 4) || (prBRefPicListInfo->u4ListIdx == 5))
      {
        //u4Temp = (((_ptFBufInfo[ucFBufIdx].i4TFldPOC <= _ptFBufInfo[ucFBufIdx].i4BFldPOC) && (_ptFBufInfo[ucFBufIdx].i4TFldPOC != 0x7fffffff))? 1: 0) << 21;
        if (ucFld == TOP_FIELD)
        {
          vVDecWriteMC(u4VDecID, RW_MC_B_LIST0 + (ucRegIdx<<2), ((UINT32) prBRefPicListInfo->u4FBufYStartAddr));
          vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_B0_RPL + (ucRegIdx<<2), (fgLRefPic << 19) + (fgLRefPic? (prBRefPicListInfo->i4TFldLongTermPicNum & 0x7ffff) : (prBRefPicListInfo->i4TFldPicNum & 0x7ffff)));

          u4Temp = ((ucFBufIdx)<<1<<22) + prBRefPicListInfo->u4TFldPara + (prBRefPicListInfo->i4TFldPOC & 0x3ffff);
          u4Temp |= (fgLRefPic << 20);
          vVDecWriteAVCMV(u4VDecID, RW_AMV_B0_REF_PARA + (ucRegIdx<<2), u4Temp);
        }
        else
        {
          vVDecWriteMC(u4VDecID, RW_MC_B_LIST0 + (ucRegIdx<<2), ((UINT32) prBRefPicListInfo->u4FBufYStartAddr));
          vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_B0_RPL + (ucRegIdx<<2), (fgLRefPic << 19) + (fgLRefPic? (prBRefPicListInfo->i4BFldLongTermPicNum & 0x7ffff) : (prBRefPicListInfo->i4BFldPicNum & 0x7ffff)));

          u4Temp = (((ucFBufIdx<<1) + 1)<<22) + prBRefPicListInfo->u4BFldPara + (prBRefPicListInfo->i4BFldPOC & 0x3ffff);
          u4Temp |= (fgLRefPic << 20);
          vVDecWriteAVCMV(u4VDecID, RW_AMV_B0_REF_PARA + (ucRegIdx<<2), u4Temp);

          vVDecWriteMC(u4VDecID, RW_AMC_B_LIST0_FLD, u4VDecReadMC(u4VDecID, RW_AMC_B_LIST0_FLD) | (0x1 << ucRegIdx));
        }
      }
      // B_1
      if ((prBRefPicListInfo->u4ListIdx == 2) || (prBRefPicListInfo->u4ListIdx == 3) || (prBRefPicListInfo->u4ListIdx == 4) || (prBRefPicListInfo->u4ListIdx == 5))
      {
        //u4Temp = (((_ptFBufInfo[ucFBufIdx].i4TFldPOC <= _ptFBufInfo[ucFBufIdx].i4BFldPOC) && (_ptFBufInfo[ucFBufIdx].i4TFldPOC != 0x7fffffff))? 1: 0) << 21;
        if (ucFld == TOP_FIELD)
        {
          vVDecWriteMC(u4VDecID, RW_MC_B_LIST1 + (ucRegIdx<<2), ((UINT32) prBRefPicListInfo->u4FBufYStartAddr));
          vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_B1_RPL + (ucRegIdx<<2), (fgLRefPic << 19) + (fgLRefPic? (prBRefPicListInfo->i4TFldLongTermPicNum & 0x7ffff) : (prBRefPicListInfo->i4TFldPicNum & 0x7ffff)));
          u4Temp = ((UINT32) prBRefPicListInfo->u4FBufMvStartAddr);
          vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + (ucRegIdx<<2), (u4Temp >> 4));

          u4Temp = ((ucFBufIdx)<<1<<22) + prBRefPicListInfo->u4TFldPara + (prBRefPicListInfo->i4TFldPOC & 0x3ffff);
          u4Temp |= (fgLRefPic << 20);
          vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + (ucRegIdx<<2), u4Temp);
        }
        else
        {
          vVDecWriteMC(u4VDecID, RW_MC_B_LIST1 + (ucRegIdx<<2), ((UINT32) prBRefPicListInfo->u4FBufYStartAddr));
          vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_B1_RPL + (ucRegIdx<<2), (fgLRefPic << 19) + (fgLRefPic? (prBRefPicListInfo->i4BFldLongTermPicNum & 0x7ffff) : (prBRefPicListInfo->i4BFldPicNum & 0x7ffff)));

          u4Temp = ((UINT32) prBRefPicListInfo->u4FBufMvStartAddr);
          vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + (ucRegIdx<<2), (u4Temp >> 4) + u4FieldDistance);

          u4Temp = (((ucFBufIdx<<1) + 1)<<22) + prBRefPicListInfo->u4BFldPara + (prBRefPicListInfo->i4BFldPOC & 0x3ffff);
          u4Temp |= (fgLRefPic << 20);
          vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + (ucRegIdx<<2), u4Temp);

          vVDecWriteMC(u4VDecID, RW_AMC_B_LIST1_FLD, u4VDecReadMC(u4VDecID, RW_AMC_B_LIST1_FLD) | (0x1 << ucRegIdx));
        }
      }
    }
    if (u4VDecReadAVCMV(u4VDecID, RW_AMV_B0_REF_PARA + (ucRegIdx<<2)) != u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + (ucRegIdx<<2)))
    {
      return TRUE;
    }
    else
    {
      return FALSE;
    }
}


// **************************************************************************
// Function : void vVDEC_HAL_H264_B1ListSwap(UINT32 u4VDecID, BOOL fgIsFrmPic);
// Description :Swap B1 reference list1
// Parameter : u4VDecID : video decoder hardware ID
//                 fgIsFrmPic : flag to frame picture or not
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_B1ListSwap(UINT32 u4VDecID, BOOL fgIsFrmPic)
{
    UINT32 u4Temp;

    if (fgIsFrmPic)
    {
        u4Temp = u4VDecReadMC(u4VDecID, RW_MC_B_LIST1);
        vVDecWriteMC(u4VDecID, RW_MC_B_LIST1, u4VDecReadMC(u4VDecID, RW_MC_B_LIST1 + 4));
        vVDecWriteMC(u4VDecID, RW_MC_B_LIST1 + 4, u4Temp);

        u4Temp = u4VDecReadAVCVLD(u4VDecID, RW_AVLD_REORD_B1_RPL);
        vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_B1_RPL, u4VDecReadAVCVLD(u4VDecID, RW_AVLD_REORD_B1_RPL + 4));
        vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_B1_RPL + 4, u4Temp);

        u4Temp = u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_PARA);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_PARA, u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + 8));
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + 8, u4Temp);

        u4Temp = u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + 4);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + 4, u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + 12));
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + 12, u4Temp);

        u4Temp = u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR, u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + 8));
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + 8, u4Temp);

        u4Temp = u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + 4);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + 4, u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + 12));
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + 12, u4Temp);
    }
    else
    {
        u4Temp = u4VDecReadMC(u4VDecID, RW_MC_B_LIST1);
        vVDecWriteMC(u4VDecID, RW_MC_B_LIST1, u4VDecReadMC(u4VDecID, RW_MC_B_LIST1 + 4));
        vVDecWriteMC(u4VDecID, RW_MC_B_LIST1 + 4, u4Temp);

        u4Temp = u4VDecReadAVCVLD(u4VDecID, RW_AVLD_REORD_B1_RPL);
        vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_B1_RPL, u4VDecReadAVCVLD(u4VDecID, RW_AVLD_REORD_B1_RPL + 4));
        vVDecWriteAVCVLD(u4VDecID, RW_AVLD_REORD_B1_RPL + 4, u4Temp);

        u4Temp = u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_PARA);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_PARA, u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + 4));
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_PARA + 4, u4Temp);

        u4Temp = u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR);
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR, u4VDecReadAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + 4));
        vVDecWriteAVCMV(u4VDecID, RW_AMV_B1_REF_ADDR + 4, u4Temp);

        u4Temp = u4VDecReadMC(u4VDecID, RW_AMC_B_LIST1_FLD) & 1; // bit 0
        // Write bit 1 to bit 0
        vVDecWriteMC(u4VDecID, RW_AMC_B_LIST1_FLD,  (u4VDecReadMC(u4VDecID, RW_AMC_B_LIST1_FLD) & (~1)) | ((u4VDecReadMC(u4VDecID, RW_AMC_B_LIST1_FLD)  >> 1) & 1));
        vVDecWriteMC(u4VDecID, RW_AMC_B_LIST1_FLD,  (u4VDecReadMC(u4VDecID, RW_AMC_B_LIST1_FLD) & (~2)) | (u4Temp << 1));
    }
}


// **************************************************************************
// Function : void vVDEC_HAL_H264_SetSPSAVLD(UINT32 u4VDecID, VDEC_INFO_H264_SPS_T *prSPS);
// Description :Set SPS data to HW
// Parameter : u4VDecID : video decoder hardware ID
//                 prSPS : pointer to sequence parameter set struct
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_SetSPSAVLD(UINT32 u4VDecID, VDEC_HAL_H264_SPS_T *prSPS)
{
    UINT32 u4SPSInfo;

    u4SPSInfo = (prSPS->u4ChromaFormatIdc & 0x3); // 1~0
    u4SPSInfo |= ((prSPS->u4Log2MaxFrameNumMinus4 & 0xf)<< 2); //5~2
    u4SPSInfo |= ((prSPS->u4PicOrderCntType & 0x3) << 6); //7~6
    u4SPSInfo |= ((prSPS->u4Log2MaxPicOrderCntLsbMinus4 & 0xf) << 8);
    u4SPSInfo |= (prSPS->fgDeltaPicOrderAlwaysZeroFlag << 12);
    u4SPSInfo |= ((prSPS->u4NumRefFrames & 0x1f) << 13);
    u4SPSInfo |= (prSPS->fgFrameMbsOnlyFlag << 18);
    u4SPSInfo |= (prSPS->fgMbAdaptiveFrameFieldFlag << 19);
    u4SPSInfo |= (prSPS->fgDirect8x8InferenceFlag << 20);
    u4SPSInfo |= (1 << 21);
    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_SPS, u4SPSInfo);
    vVDecWriteVLDTOP(u4VDecID, RW_VLD_PIC_MB_SIZE_M1, ((prSPS->u4PicHeightInMapUnitsMinus1) << VLD_TOP_PIC_HEIGHT_IN_MBS_POS) | prSPS->u4PicWidthInMbsMinus1);
}


// **************************************************************************
// Function : void vVDEC_HAL_H264_SetPPSAVLD(UINT32 u4VDecID, BOOL fgUserScalingMatrixPresentFlag,
//    BOOL *fgUserScalingListPresentFlag, VDEC_INFO_H264_PPS_T *prVDecH264PPS);
// Description :Set PPS data to HW
// Parameter : u4VDecID : video decoder hardware ID
//                 prPPS : pointer to picture parameter set struct
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_SetPPSAVLD(UINT32 u4VDecID, BOOL fgUserScalingMatrixPresentFlag,
    BOOL *pfgUserScalingListPresentFlag, VDEC_HAL_H264_PPS_T *prVDecH264PPS)
{
    UINT32 u4PPSInfo;
    INT32 i;

    u4PPSInfo = prVDecH264PPS->fgEntropyCodingModeFlag;
    u4PPSInfo |= (prVDecH264PPS->fgPicOrderPresentFlag << 1);
    u4PPSInfo |= (prVDecH264PPS->fgWeightedPredFlag << 2);
    u4PPSInfo |= ((prVDecH264PPS->u4WeightedBipredIdc & 0x03) << 3);
    u4PPSInfo |= ((prVDecH264PPS->i4PicInitQpMinus26 & 0x3f) << 5);
    u4PPSInfo |= ((prVDecH264PPS->i4ChromaQpIndexOffset & 0x1f)<< 11);
    u4PPSInfo |= (prVDecH264PPS->fgDeblockingFilterControlPresentFlag << 16);
    u4PPSInfo |= (prVDecH264PPS->fgConstrainedIntraPredFlag << 17);
    u4PPSInfo |= (prVDecH264PPS->fgTransform8x8ModeFlag << 18);
    u4PPSInfo |= ((prVDecH264PPS->i4SecondChromaQpIndexOffset & 0x1f) << 19);

    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_PPS_1 , u4PPSInfo);

    u4PPSInfo = prVDecH264PPS->u4NumRefIdxL0ActiveMinus1;
    u4PPSInfo |= (prVDecH264PPS->u4NumRefIdxL1ActiveMinus1 << 5);
    for (i=0; i<8; i++)
    {
      u4PPSInfo |= (pfgUserScalingListPresentFlag[i] << (10 + i));
    }
    u4PPSInfo |= (fgUserScalingMatrixPresentFlag << 18);

    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_PPS_2 , u4PPSInfo);
}


// **************************************************************************
// Function : void vVDEC_HAL_H264_SetSHDRAVLD1(UINT32 u4VDecID, VDEC_INFO_H264_SLICE_HDR_T *prSliceHdr);
// Description :Set part of slice header data to HW
// Parameter : u4VDecID : video decoder hardware ID
//                 prSliceHdr : pointer to slice parameter set struct
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_SetSHDRAVLD1(UINT32 u4VDecID, VDEC_HAL_H264_SLICE_HDR_T *prSliceHdr)
{
    UINT32 u4SHDRInfo;

    u4SHDRInfo = prSliceHdr->u4FirstMbInSlice & 0x1fff;
    u4SHDRInfo |= ((prSliceHdr->u4SliceType & 0xf) << 13);
    u4SHDRInfo |= (prSliceHdr->fgFieldPicFlag << 17);
    u4SHDRInfo |= (prSliceHdr->fgBottomFieldFlag << 18);
    u4SHDRInfo |= (prSliceHdr->fgDirectSpatialMvPredFlag << 19);
    u4SHDRInfo |= ((prSliceHdr->u4NumRefIdxL0ActiveMinus1 & 0x1f) << 20);
    u4SHDRInfo |= ((prSliceHdr->u4NumRefIdxL1ActiveMinus1 & 0x1f) << 25);

    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_PIC_SIZE, (u4VDecReadAVCVLD(u4VDecID, RW_AVLD_PIC_SIZE) &(~0x1FFFF)) | (u4SHDRInfo&0x1FFF));
    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_SHDR_1, (u4VDecReadAVCVLD(u4VDecID, RW_AVLD_SHDR_1) &(~0xFFFFE000)) | (u4SHDRInfo&0xFFFFE000));
}

// **************************************************************************
// Function : INT32 i4VDEC_HAL_H264_DecStart(UINT32 u4VDecID, VDEC_INFO_DEC_PRM_T *prDecPrm);
// Description :Set video decoder hardware registers to decode for H264
// Parameter : ptHalDecH264Info : pointer to H264 decode info struct
// Return      : =0: success.
//                  <0: fail.
// **************************************************************************
INT32 i4VDEC_HAL_H264_DecStart(UINT32 u4VDecID, VDEC_HAL_H264_DEC_PRM_T *prHalH264DecPrm)
{
    UINT32 u4SHDRInfo;
    UINT32 u4RegValue;
    UINT32 u4CRCSrc = 0x1;

#if VDEC_DDR3_SUPPORT
    UINT32 u4DDR3_PicWdith;
    UINT32 aMc406;
#endif

#if (!CONFIG_DRV_VERIFY_SUPPORT)
    UINT32  aAVCVLD84 = 0;
#endif
    UINT32 u4total_mbs_in_pic = 0;
    UINT32 u4FieldDistance = 4;

   // printk("\n");

    u4CRCSrc = (VDEC_CRC_EN | VDEC_CRC_SRC_PP);  //CRC input from PP
    vVDecWriteCRC(u4VDecID, 0x4, u4CRCSrc);

    // addr swap mode
    vVDecWriteMC(u4VDecID, RW_MC_ADDRSWAP, prHalH264DecPrm->ucAddrSwapMode);

    // set MC wait timeout threshold
    vVDecWriteVLDTOP(u4VDecID, RW_VLD_TOP_BUSY_THRESHOLD, 0xFFFFFFFF);
    // Fld & Frame height same?
    vVDecWriteMC(u4VDecID, RW_MC_UMV_PIC_WIDTH, prHalH264DecPrm->u4PicW);
    vVDecWriteMC(u4VDecID, RW_MC_UMV_PIC_HEIGHT, prHalH264DecPrm->u4PicH);

    // MT3363 add by wc
    u4RegValue =  prHalH264DecPrm->u4PicH << 16 | prHalH264DecPrm->u4PicW;
    vVDecWriteVLDTOP(u4VDecID, RW_VLD_TOP_PIC_PIX_SIZE, u4RegValue);


    if(prHalH264DecPrm->ucECLevel == 0)
    {
         // Default No EC, Mask for selected types
        vVDecWriteAVCVLD(u4VDecID, RW_AVLD_ERR_MASK, (~(CABAC_ALIGN_BIT_ERR | CABAC_ZERO_WORD_ERR | AVLD_4BLOCKS_SKIP_CHK | NO_NEXT_START_CODE)));
    }
    else
    {
        //vVDecWriteAVCVLD(u4VDecID, RW_VLD_AV_CTRL, u4VDecReadAVCVLD(u4VDecID, RW_VLD_AV_CTRL) | AVC_ERR_CONCEALMENT);
        aAVCVLD84 = (u4VDecReadAVCVLD(u4VDecID, RW_VLD_AV_CTRL) | AVC_ERR_CONCEALMENT);
        aAVCVLD84 |= AVC_SUM6_APPEND_INV;
        aAVCVLD84 |= AVC_NOT_CHK_DATA_VALID;

       vVDecWriteAVCVLD(u4VDecID, RW_VLD_AV_CTRL, aAVCVLD84);

        vVDecWriteAVCVLD(u4VDecID, RW_AVLD_ERR_MASK,  (~(CABAC_ALIGN_BIT_ERR | CABAC_ZERO_WORD_ERR | AVLD_4BLOCKS_SKIP_CHK | NO_NEXT_START_CODE)));
    }

    // Only one case needs to turn off deblocking
    if (0)//prH264DecPrm->prSliceHdr->u4DisableDeblockingFilterIdc == 1)
    {
        //test only
        vVDecWriteMC(u4VDecID, RW_MC_PP_ENABLE, 0);
        vVDecWriteMC(u4VDecID, RW_MC_PP_WB_BY_POST, 0);
        vVDecWriteMC(u4VDecID, RW_MC_PP_DBLK_MODE, 0);
    }
    else
    {
        vVDecWriteMC(u4VDecID, RW_MC_PP_ENABLE, 1);
        vVDecWriteMC(u4VDecID, RW_MC_PP_Y_ADDR, ((UINT32) prHalH264DecPrm->u4YStartAddr) >> 9);
        vVDecWriteMC(u4VDecID, RW_MC_PP_C_ADDR, ((UINT32) (prHalH264DecPrm->u4YStartAddr + prHalH264DecPrm->u4CAddrOffset)) >> 8);
        vVDecWriteMC(u4VDecID, RW_MC_PP_WB_BY_POST, 1);
        vVDecWriteMC(u4VDecID, RW_MC_PP_DBLK_MODE, DBLK_Y | DBLK_C);
        vVDecWriteMC(u4VDecID, RW_MC_PP_X_RANGE, ((prHalH264DecPrm->u4PicW + 15)>> 4) - 1);
        vVDecWriteMC(u4VDecID, RW_MC_PP_Y_RANGE, (((prHalH264DecPrm->u4PicH >> (1-(prHalH264DecPrm->fgIsFrmPic))) + 15)>> 4) - 1);
        vVDecWriteMC(u4VDecID, RW_MC_PP_MB_WIDTH, ((prHalH264DecPrm->u4PicW + 15)>> 4));
    }


    vVDecWriteMC(u4VDecID, RW_AMC_Y_OUT_ADDR, ((UINT32) prHalH264DecPrm->u4YStartAddr));
    vVDecWriteMC(u4VDecID, RW_AMC_CBCR_OFFSET, prHalH264DecPrm->u4CAddrOffset);

  vVDecWriteMC(u4VDecID, RW_MC_PIC_W_MB, ((prHalH264DecPrm->u4PicBW + 15)>> 4));

  // Turn off find start code function
  vVDecWriteAVCVLD(u4VDecID, RW_AVLD_RM03R, ((u4VDecReadAVCVLD(u4VDecID,  RW_AVLD_RM03R)) & 0xFFFFFFDF) );


    vVDecWriteAVCMV(u4VDecID, RW_AMV_WR_ADDR,
                          ((((UINT32) prHalH264DecPrm->u4MvStartAddr) >> 4)) + ((prHalH264DecPrm->ucPicStruct == BOTTOM_FIELD)? u4FieldDistance : 0));

    //SetSHDRAVLD2
    u4SHDRInfo = (prHalH264DecPrm->rSliceHALParam.i4SliceQpDelta) & 0x7f;
    u4SHDRInfo |= ((prHalH264DecPrm->rSliceHALParam.u4DisableDeblockingFilterIdc & 0x3) << 7);
    u4SHDRInfo |= ((prHalH264DecPrm->rSliceHALParam.i4SliceAlphaC0OffsetDiv2 & 0xf) << 9);
    u4SHDRInfo |= ((prHalH264DecPrm->rSliceHALParam.i4SliceBetaOffsetDiv2 & 0xf) << 13);
    u4SHDRInfo |= ((prHalH264DecPrm->ucNalRefIdc & 0x3) << 17);
    u4SHDRInfo |= (prHalH264DecPrm->fgIsIDRPic << 19);
    u4SHDRInfo |= ((prHalH264DecPrm->rSliceHALParam.u4CabacInitIdc & 0x3) << 20);

    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_SHDR_2, u4SHDRInfo);
    if (prHalH264DecPrm->rPPSHALParam.fgEntropyCodingModeFlag) // CABAC only
    {
        vVDecWriteAVCVLD(u4VDecID, RW_AVLD_INIT_CTX_SRAM, 0x1);
    }

    while (u4VDecReadAVCVLD(u4VDecID, RO_AVLD_STATUS) & RO_AVLD_STALL);

    vVDecWriteMC(0, 0x5E4, (u4VDecReadMC(0, 0x5E4) |(0x1 <<12)) );
    vVDecWriteMC(0, 0x660, (u4VDecReadMC(0, 0x660) |(0x80000000)) );
    #ifndef VDEC_PIP_WITH_ONE_HW
    vVDecWriteMC(1, 0x5E4, (u4VDecReadMC(1, 0x5E4) |(0x1 <<12)) );
    vVDecWriteMC(1, 0x660, (u4VDecReadMC(1, 0x660) |(0x80000000)) );
    #endif

     vVDecWriteMC(u4VDecID, RW_MC_NBM_CTRL,
             ((u4VDecReadMC(u4VDecID, RW_MC_NBM_CTRL)  & 0xFFFFFFF8) | prHalH264DecPrm->ucAddrSwapMode));
// turn off test mode 0x834[4]
     vVDecWriteMC(u4VDecID, RW_MC_DDR3_EN, (u4VDecReadMC(u4VDecID, RW_MC_DDR3_EN)  & 0xFFFFFFEF));

    if (prHalH264DecPrm->rSPSHALParam.fgFrameMbsOnlyFlag == 0 && prHalH264DecPrm->rSliceHALParam.fgFieldPicFlag == 0)
    {
        u4total_mbs_in_pic = ((prHalH264DecPrm->rSPSHALParam.u4PicHeightInMapUnitsMinus1<<1) + 2)*(prHalH264DecPrm->rSPSHALParam.u4PicWidthInMbsMinus1 + 1);
    }
    else
    {
        u4total_mbs_in_pic = (prHalH264DecPrm->rSPSHALParam.u4PicHeightInMapUnitsMinus1 + 1)*(prHalH264DecPrm->rSPSHALParam.u4PicWidthInMbsMinus1 + 1);
    }

    vVDecWriteVLDTOP(u4VDecID, RW_VLD_TOP_TOTAL_MBS_IN_PIC, u4total_mbs_in_pic);
    vVDecWriteVLDTOP(u4VDecID, RW_VLD_TOP_TIMEOUT_THD, VLD_TOP_TIMEOUT_THD);

    u4CRCSrc = 0x10;
    vVDecWriteCRC(u4VDecID, VDEC_CRC_REG_EN, u4CRCSrc);

    #ifdef MPV_DUMP_H264_DEC_REG
    //vVDEC_HAL_H264_VDec_DumpReg(u4VDecID);
    #endif
    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_PROC, 0x1);

    return HAL_HANDLE_OK;
}


// **************************************************************************
// Function : void v4VDEC_HAL_H264_GetMbxMby(UINT32 u4VDecID, UINT32 *pu4Mbx, UINT32 *pu4Mby);
// Description :Read current decoded mbx and mby
// Parameter : u4VDecID : video decoder hardware ID
//                 u4Mbx : macroblock x value
//                 u4Mby : macroblock y value
// Return      : None
// **************************************************************************
void vVDEC_HAL_H264_GetMbxMby(UINT32 u4VDecID, UINT32 *pu4Mbx, UINT32 *pu4Mby)
{
    *pu4Mbx = u4VDecReadMC(u4VDecID, RO_MC_MBX);
    *pu4Mby = u4VDecReadMC(u4VDecID, RO_MC_MBY);
}


UINT32 u4VDEC_HAL_H264_VDec_ReadFinishFlag(UINT32 u4VDecID)
{
  return u4VDecReadAVCVLD(u4VDecID, RO_AVLD_COMPLETE);
}

//***********************
/*For common HAL interface*/
//***********************
//
//ucEsId can not get from Hal, maitain one HalParam
//but reserve "ucEsId"
VDEC_HAL_H264_DEC_PRM_T *VDec_GetH264HalParam(UCHAR ucEsId)
{
    if(ucEsId >= 2)
    {
        //printk("Get Null pointer @ %s\n",__FUNCTION__);
        return NULL;
    }
    else
    {
        return &_arH264HalDecParam;
    }
}
