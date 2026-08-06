/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER
AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */


/*******************************************************************************
*
* Filename:
* ---------
* file SPDec.h
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*
*
* Author:
* -------
*
*
*------------------------------------------------------------------------------
* $Revision: #10 $
* $Modtime:$
* $Log:$
*
*******************************************************************************/

#ifndef SPDEC_H_
#define SPDEC_H_


#include "linux/types.h"
#include "stdbool.h"
#include "mm_errcode.h"


typedef void   *HSPDECINST;

#ifdef __cplusplus
extern "C" {
#endif

#define INDEXMODEL            1
#define BMP_PALETTE_SIZE      ((__u32)32)

#define RET_RLE_OUT_OF_MEM_RANGE              RET_MSDKC_OUTOFMEMORY

#define RLE_SW_DEBUG_LOG_ON             0

#define RLE_PIC_LINE_MAX_WIDTH          ((__u32)3000)
#define RLE_SW_GET_BITS_WARING_NUM      ((__u32)25)
#define RLE_SW_GET_BITS_WORD_BITS       ((__u32)32)
#define RLE_SW_RUN_2BITS_LSEFT_MAX      ((__u32)0x40)
#define RLE_SW_RUN_2BITS_LSHIFT_BIT     ((__u32)2)
#define RLE_SW_RUN_2BITS_GET_BITS       ((__u32)4)
#define RLE_SW_RUN_2BITS_COLOR_MASK     ((__u32)3)
#define RLE_SW_RUN_2BITS_COLOR_BITS     ((__u32)2)
#define RLE_SW_4BIT_ALGIN_MASK          ((__u32)7)
#define RLE_SW_4BIT_ALGIN_SHIFT_BIT     ((__u32)3)

#define RLE_SW_READ_BIT32(x) (((__u32)((const __u8*)(x))[0] << 24) |      \
                              ((__u32)((const __u8*)(x))[1] << 16) |    \
                              ((__u32)((const __u8*)(x))[2] <<  8) |    \
                              (__u32)((const __u8*)(x))[3])

#define RLE_SW_READ_32BITS(pBuffer, u4BitIdx)   \
    (RLE_SW_READ_BIT32((pBuffer) + ((__u32)(u4BitIdx) >> RLE_SW_4BIT_ALGIN_SHIFT_BIT)) << ((__u32)(u4BitIdx) & RLE_SW_4BIT_ALGIN_MASK))


typedef struct _RLEPicPosInfo
{
    __u32 u4StartX;
    __u32 u4StartY;
    __u32 u4EndX;
    __u32 u4EndY;
} RLEPicPosInfo;

typedef struct _RLESWDecInfo
{
    __u8 *pu1WorkBufVA;
    __u32 u4WorkBufSize;

    __u8 *pu1PicTopBuf;
    __u8 *pu1PicBottomBuf;
    __u8 *pu1PicBufEnd;

    __u32 u4PicWidth;
    __u32 u4PicHigh;
    __u32 u4Pitch;

    RLEPicPosInfo rPicPos;
} RLESWDecInfo;

typedef struct _GetBitContext
{
    __u8 *pu1Buffer;
    __u8 *pu1BufferEnd;
    __u32 u4Index;
    __u32 u4SizeInBits;
    __u32 u4SizeInBitsPlus8;
} GetBitContext;

MRESULT mrSPDecode(RLESWDecInfo *pRLESWDecInfoT);

typedef struct
{
    __u32 sx;
    __u32 sy;
    __u32 ex;
    __u32 ey;
    __u32 u4Pitch;
    void *pvData;
    __u32  u4Size;
    __u64  u8Pts;
    __u64  u8Duration;
} RLE_DISPLAY_INFO;

typedef struct
{
    __u32 sx;
    __u32 sy;
    __u32 u4PicWidth;
    __u32 u4PicHigh;
    void *pvData;
    __u32  u4Size;
    __u64  u8Pts;
    __u64  u8Duration;
} DivXBtnDecInfo;

typedef struct
{
    __u32  u4Width;
    __u32  u4Height;
} SUBTITLE_RESOLUTION;

typedef struct
{
    void  *pWorkBuf;
    __u32  u4DstPitch;
    __u32  u4BMPHeaderSize;
    __u32  u4BufferSize;
    __u32  u4Width;
    __u32  u4Height;
} SpSWDecIns;

typedef struct
{
    __u32 u4SX;
    __u32 u4SY;
    __u32 u4Width;
    __u32 u4Height;
    char   achPAL[BMP_PALETTE_SIZE];
    void  *pvData;
    __u32 u4BufSize;
} SpSWDecResult;

MRESULT SPSWDec_Decode(void *srcdata, __u32 datalen, void *pvSampleData, SpSWDecResult *pDisplayInfo, void *data);
void SPSWDec_RelRes(SpSWDecResult *pDisplayInfo);
#if 0
bool DivXBtnDecode(void *pvSampleData, DivXBtnDecInfo *pDisplayInfo);
#endif
void DivXBtnDecRelease(DivXBtnDecInfo *pDisplayInfo);

void SPDec_DumpEx(int fd, char args[][40], __u32 u4len);

#ifdef __cplusplus
}
#endif

#endif //_AVDMX_H_
