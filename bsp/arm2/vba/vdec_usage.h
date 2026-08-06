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
#ifndef _VDEC_USAGE_H_
#define _VDEC_USAGE_H_

#include "x_typedef.h"
#include "vdec_info_common.h"


#define VDEC_MODE_NAME                   "VDEC"
#define VDEC_VER_MAJOR                   03
#define VDEC_VER_MINOR                   01
#define VDEC_VER_REV                     00

#define OSR_NO_MSG                  ((INT32) -13)
typedef enum
{
    VDEC_UNKNOWN     = 0xFF,
    VDEC_H264        = 0x264,           ///< H264 Deocde Request
} VDEC_CODEC_T;



typedef struct _VDEC_FBM_INFO_T
{
    UCHAR ucFbgId;
    UCHAR ucFbgFbNum;
    UCHAR ucFbgDpbNum;
    UCHAR ucFbId;
    UCHAR ucFbDbId;
    UCHAR ucVDSCLFbId;
    UINT32 u4VDSCLWorkAddr;
    UINT32 u4VVDSCLWorkAddr;
    UINT32 u4FbgWidth;
    UINT32 u4FbgHeight;
    UINT32  u4FbgAllocWidth;
    UINT32 u4WaitDispTime;		//    UINT32 u4WaitDispTime;
    UINT32  u4VdsclFbWidth;
} VDEC_FBM_INFO_T;


typedef struct _VDEC_PIC_INFO_T
{
    UINT32  u4PicType;
    UINT32  u4PicWidth;
    UINT32  u4PicHeight;
    UINT32  u4LastPicWidth;
    UINT32  u4LastPicHeight;
    UINT32  u4PicAllocWidth;
    UINT32  u4PicAllocHeight;
    UINT64  u8DTS;
    UINT64  u8PTS;
    UINT64  u8LastPts;
    UINT64  u8Offset;
    UINT32  u4DecAddrY;
    UINT32  u4DecAddrC;
    UINT32  u4DecAddrA;
    UINT32  u4VirAddrY;
    UINT32  u4VirAddrC;
    UINT32  u4VdsclAddrY;
    UINT32  u4VdsclAddrC;
    UINT32  u4FifoStart;
    UINT32  u4FifoEnd;
    UINT32  u4VFifoStart;
    UINT32  u4VFifoEnd;
    UINT32  u4AUStart;
    UINT32  u4AUEnd;
    UINT32  u4VldReadPtr;
    UINT32  u4VldWritePtr;
    UINT32  u4VVldReadPtr;
    UINT32  u4PicEnd;
    UINT32  u4VC1CCSa;
    UINT32  u4CCIdx;
    UINT32  u4PicFlag;
    BOOL    fgVirtualDec;
    BOOL    fgForceVirtual;
    UINT32  u4Duration;
    UINT32  u4WMVSliceAddr[3];

    UINT32  u4xvColorR;
    UINT32  u4xvColorG;
    UINT32  u4xvColorB;

    UINT32  u4RMSliceNum;
    UINT32  auRM4SliceSize[128];

    UINT32  u4EsdIndex;

    UINT32  u4EsdNums;
    UCHAR   ucPicStruct;
    BOOL    fgDec2ndFld; // frame: false
} VDEC_PIC_INFO_T;

typedef struct _VDEC_NORM_INFO_T
{
    VDEC_CODEC_T   eVdecCodecType;
    UCHAR  ucEsId;
    UCHAR  ucVldId;
    UCHAR  ucBsId;
    UCHAR  ucAddrSwapMode;
    BOOL fgDeblocking;
    BOOL fgDeblockingDemo;
    UINT8 u1DeblockLevel;
    UINT8 au1MBqp[4];
    BOOL fgForceDropLevl;
    BOOL fgErrRefExist;
    UCHAR  ucPicAsp;
    UCHAR  ucFrameRate;
    UCHAR ucLastPicTp;
    UCHAR ucLastPicStruct;
    BOOL fgSeqChg;
    UCHAR  ucHScale;
    UCHAR  ucVScale;
    UINT16 u2LineSize;				// horizontal line size, due to block color mode
    UINT64  u8LatestDecPTS;
    UINT64  u8LatestOutPTS;
    UINT64  u8LastIPTS;
    UINT32  u8LastIOffset;
    UINT32  u4LatestFrameRate;
    UINT32 u4DisplayRef;

    UINT32  u4PicSize;
    UINT32  u4SampleWidth;
    UINT32  u4SampleHeight;
    UINT32  u4FrameTimingInfo;
    UINT32  u4FrameDuration;
    UINT32  u4ColorPrimaries;

    UINT32  u4DecStatus;
    UINT32  u4DecFlag;
    UINT32  u4EventRec;
    UINT32 u4DecReadPtr;

    UINT32 u4VParseErrCode;
    INT32 i4VDecodeErrCode;
    UINT32 u4PrsHdrType;
    UINT32 u4RefFrmCnt;

    UINT32 u4DropPNum;
    BOOL fgFFShow;

    INT32  i4RemainNum;
    UINT32  u4ExpRemNum;

    UINT32  u4FFDivisor;
    UINT32  u4FFRemainder;
    UINT32  u4FFTargetCnt;
    UINT32  u4FFDivisorCnt;
    UINT32  u4FFRemainderCnt;

    UINT64 u8PreRefPTS;

    BOOL    fgIsProgressive;
    BOOL   fgIsVDPRacing;
    BOOL    fgRepFirstFld;
    BOOL    fgBPicInRA;
} VDEC_NORM_INFO_T;

typedef struct _VDEC_ES_INFO_T
{
    BOOL   fgSupportDI; // support deinterlace
    BOOL   fgIsOneSeg;//OneSeg flag
    BOOL   fgECEnable;//Error Concealment Enable Flag
    BOOL   fgIsWFD;
    UINT32  u4PicCnt;
    UINT32  u4OutputPicCnt;
    VDEC_NORM_INFO_T   rVDecNormInfo;
    VDEC_PIC_INFO_T   rVDecPrevPicInfo;
    VDEC_PIC_INFO_T   rVDecCurrPicInfo;
    BOOL fgIsNextPicInfoExisted;
    VDEC_PIC_INFO_T   rVDecNextPicInfo;
    VDEC_FBM_INFO_T    rVDecFbmInfo;
    UINT32 *prVDecDrvInfo;
    BOOL fgReDecode;
    UCHAR ucReDecCnt;
    UINT32 u4TotalReDecCnt;
    BOOL fgNoSc;
    UINT32 u4AlphaFlag;
} VDEC_ES_INFO_T;

#endif  //_VDEC_USAGE_H_

