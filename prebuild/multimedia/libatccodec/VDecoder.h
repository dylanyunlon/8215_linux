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
* file VDecoder.h
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*the header file of VDecoder
*
* Author: Jianhua Feng
* -------
*
*
*------------------------------------------------------------------------------
* $Revision: #3 $
* $Modtime:$2010-4-14
* $Log:$
*
*******************************************************************************/
#ifndef VDECODER_
#define VDECODER_
typedef __signed__ int __s32;
typedef unsigned int __u32;

#include <stdbool.h>
#include <pthread.h>

#include "mm_common.h"
#include "vdec_init.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef void * HAVDECINST;

typedef enum _PARMTYPE
{
    APARAM_NONE_T,
    APARAM_PCM_T,
    APARAM_COOK_T,
    APARAM_WMA_T,
    APARAM_APE_T,
    APARAM_FLAC_T,
    APARAM_VOLUME_T,
    APARAM_GET_EOS,
    APARAM_GET_SPECTRUM,
    APARAM_ORIG_SAMPRATE,
    APARAM_SET_SE,
    APARAM_SET_AC3DRC,
    APARAM_SET_DTSDRC,
    APARAM_FEATURE,
    APARAM_BMANAGEMENT,
    APARAM_GET_TARGETPTS,
    APARAM_SET_TARGETPTS,
    APARAM_DISABLE_AVSYNC,
    APARAM_GETCURPLAYTIME,
    APARAM_SET_STC_VALID,
    APARAM_GET_VOLUME_T,
    APARAM_SET_REAR_VOLUME_T,
    APARAM_GET_REAR_VOLUME_T,
    APARAM_AUD_MUTE,
    VPARAM_SET_VDEC_PROG_TYPE,
    VPARAM_GET_VIDEO_ASPECT_RATIO,
    APARAM_MIRACAST_T,
    APARAM_SET_WRITE_T,
    APARAM_SET_AUDDEC_INFO_T,
    PARAM_MAX_T,
}ParamType;

typedef struct _VIDEO_DECODER_
{
    VDEC_CODEC_T eVCodec;
    void*   pVdecDrv;
    pthread_mutex_t m_mutex;
} VIDEO_DECODER;

VIDEO_DECODER* VDec_CreateInstance(AVCODECID_T codec_type, __u32 u4Flag);
void VDec_Release(HAVDECINST hInst);
bool VDec_SetParam(HAVDECINST hInst, ParamType eParamType, void *prParam);
void VDec_GetParam(HAVDECINST hInst, ParamType eParamType, void *prParam);
bool VDec_SetInputBuf(HAVDECINST hInst, void *pvBuf, __u32 u4BufSz, void *pvOutBuf, __u32 u4OutBufSz);
void VDec_SetOutputBuf(HAVDECINST hInst, void *pvBuf, __u32 u4BufSz);
bool VDec_GetOutputBuf(HAVDECINST hInst, void* pvBuf);
bool VDec_Stop(HAVDECINST hInst, __u32* out);

#ifdef __cplusplus
}
#endif

#endif