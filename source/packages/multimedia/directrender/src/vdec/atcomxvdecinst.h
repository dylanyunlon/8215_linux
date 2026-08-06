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
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
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

#ifndef _ATC_3RDPARTY_VDEC_INST_H_
#define _ATC_3RDPARTY_VDEC_INST_H_

#include <OMX_Video.h>
#include "OMX_ComponentExt.h"

typedef enum VIDEO_PROGRAM_TYPE{
  VIDEO_PROGRAM_UNKNOWN,
  VIDEO_PROGRAM_WELINK,
  VIDEO_PROGRAM_CARLIFE,
  VIDEO_PROGRAM_CARPLAY,
  VIDEO_PROGRAM_MIRACAST,
  VIDEO_PROGRAM_DVR,
  VIDEO_PROGRAM_FILE,
}VIDEO_PROGRAM_TYPE;

typedef struct {
  OMX_VIDEO_CODINGTYPE eVType; /* video codec type */
  __s32 width;        /* video width */
  __s32 height;       /* video height */
  __s32 fps_n;        /* numerator of video framerate */
  __s32 fps_d;        /* denominator of video framerate */
  bool  interlaced;   /* designate whether video data is interaced  */
  VIDEO_PROGRAM_TYPE eVProgramType;    /* video program type */
} ATC_VDEC_INPUT_FMT_INFO_T;

typedef struct _AtcVdecBufferInfo{
  void *buffer;       /* output buffer  */
  __u32 bufSz;        /* output buffer size */
  __u32 datasz;       /* valid data size in output buffer  */
  __s32 width;        /* video width */
  __s32 height;       /* video height */
  __s64 timestampus;  /* video frame timestamp, unit: us */
  __u32 flags;
} VDEC_BUFFER_INFO_T;

typedef enum {
  RET_ATCVDECINST_OK,            /* success */
  RET_ATCVDECINST_PARAM_ERR,     /* parameter error */
  RET_ATCVDECINST_NO_MEM,        /* no memory */
  RET_ATCVDECINST_WRONG_STATE,   /* state is error */
  RET_ATCVDECINST_COMPONENT_ERR, /* omx component encounter error */
  RET_ATCVDECINST_FLUSHING,      /* in flushing */
  RET_ATCVDECINST_FAIL,          /* other error */
} ATC_VDEC_ERRCODE;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BUFFER_CALLBACKTYPE
{
    void (*onEmptyBufferDone)(
        void *pAppData,
        VDEC_BUFFER_INFO_T* pBuffer);

    void (*onFillBufferDone)(
        void *pAppData,
        VDEC_BUFFER_INFO_T* pBuffer);
} BUFFER_CALLBACKTYPE;

void *atc_vdec_open(void);

bool  atc_vdec_set_input_format(void *inst, ATC_VDEC_INPUT_FMT_INFO_T *format);

bool  atc_vdec_get_output_formats(void *inst,
  __u64 *psupport_fmts, __u32 *pformat);

void atc_vdec_set_callbacks(void *inst, BUFFER_CALLBACKTYPE *callbacks, void *app_data);

bool  atc_vdec_decode(void *inst, __u8 *pdata, __u32 datasz, __s64 timestampus, __u32 flags);

bool  atc_vdec_close(void *inst);

bool  atc_vdec_start(void *inst);

bool  atc_vdec_flush(void *inst);

ATC_VDEC_ERRCODE atc_vdec_release_output_buffer(void * inst,
  VDEC_BUFFER_INFO_T *prOutInfo);


bool  atc_vdec_stop(void *inst);

#ifdef __cplusplus
}
#endif


#endif /* _ATC_3RDPARTY_VDEC_INST_H_ */

